/*
 * Copyright (C) ST-Ericsson SA 2010
 * Author: Pierre Peiffer <pierre.peiffer@stericsson.com>
 * for ST-Ericsson.
 * Philippe Langlais <philippe.Langlais@stericsson.com> for ST-Ericsson.
 * License terms: GNU General Public License (GPL), version 2.
 */

#include <linux/io.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <trace/stm.h>
#ifdef CONFIG_BCM_STM
#include "linux/broadcom/mobcom_types.h"
#include "mach/rdb/brcm_rdb_padctrlreg.h"
#include "mach/rdb/brcm_rdb_util.h"
#include "mach/rdb/brcm_rdb_chipreg.h"
#include <mach/hardware.h>
#include <plat/chal/chal_trace.h>
#include <mach/io_map.h>
#endif
#include <mach/clock.h>

/* define to disable PTI for ETM and PTM */
#define PTI_OFF	0x013
/* Max number of channels (multiple of 256) */
#define STM_NUMBER_OF_CHANNEL      CONFIG_STM_NUMBER_OF_CHANNEL

/* # dynamically allocated channel with stm_trace_buffer */
#define NB_KERNEL_DYNAMIC_CHANNEL  128

static struct stm_device {
	const struct stm_platform_data *pdata;
	/* Used to register the allocated channels */
	 DECLARE_BITMAP(ch_bitmap, STM_NUMBER_OF_CHANNEL);
} stm;

static struct cdev cdev;
static struct class *stm_class;
static int stm_major;
static struct clk *pti_peri_clk;

#define STM_BUFSIZE    256
struct channel_data {
	DECLARE_BITMAP(bitmap, STM_NUMBER_OF_CHANNEL);
	int numero;
	spinlock_t lock;
	u8 data_buffer[STM_BUFSIZE];
};

static u64 stm_printk_buf[1024 / sizeof(u64)];
static DEFINE_SPINLOCK(stm_buf_lock);

#ifdef CONFIG_BCM_STM
/* STM and SWSTM on A9 will be initialized at boot loader. */
#define ATB_ID_ODD(x) ((x<<1)|0x1)	/* 6bit ATB ID + 1bit (1) */
#define ATB_ID_EVEN(x) (x<<1)	/* 6bit ATB ID + 1bit (0) */
#define INVALID_FUNNEL CHAL_TRACE_MAX_FUNNEL
#define LOCK_ACCESS_CODE 0xC5ACCE55

static CHAL_TRACE_DEV_t trace_base_addr = {
	.CHIPREGS_base = KONA_CHIPREG_VA,
	.PWRMGR_base = KONA_PWRMGR_VA,
	.FUNNEL_base[CHAL_TRACE_HUB_FUNNEL] = KONA_HUB_FUNNEL_VA,
	.FUNNEL_base[CHAL_TRACE_FUNNEL] = KONA_FUNNEL_VA,
	.FUNNEL_base[CHAL_TRACE_FIN_FUNNEL] = KONA_FIN_FUNNEL_VA,
	.FUNNEL_base[CHAL_TRACE_FAB_FUNNEL1] = KONA_FAB_FUNNEL1_VA,
	.FUNNEL_base[CHAL_TRACE_COMMS_FUNNEL] = KONA_BMODEM_FUNNEL_VA,
	.AXITRACE_base[CHAL_TRACE_AXITRACE1] = KONA_AXITRACE1_VA,
	.AXITRACE_base[CHAL_TRACE_AXITRACE2] = KONA_AXITRACE2_VA,
	.AXITRACE_base[CHAL_TRACE_AXITRACE3] = KONA_AXITRACE3_VA,
	.AXITRACE_base[CHAL_TRACE_AXITRACE4] = KONA_AXITRACE4_VA,
	.AXITRACE_base[CHAL_TRACE_AXITRACE11] = KONA_AXITRACE11_VA,
	.AXITRACE_base[CHAL_TRACE_AXITRACE12] = KONA_AXITRACE12_VA,
	.AXITRACE_base[CHAL_TRACE_AXITRACE16] = KONA_AXITRACE16_VA,
	.AXITRACE_base[CHAL_TRACE_AXITRACE17] = KONA_AXITRACE17_VA,
	.AXITRACE_base[CHAL_TRACE_AXITRACE18] = KONA_AXITP18_VA,
	.AXITRACE_base[CHAL_TRACE_AXITRACE19] = KONA_AXITRACE19_VA,
	.CTI_base[CHAL_TRACE_HUB_CTI] = KONA_HUB_CTI_VA,
	.CTI_base[CHAL_TRACE_MM_CTI] = KONA_MM_CTI_VA,
	.CTI_base[CHAL_TRACE_FAB_CTI] = KONA_FAB_CTI_VA,
	.CTI_base[CHAL_TRACE_A9CTI0] = KONA_A9CTI0_VA,
	.CTI_base[CHAL_TRACE_R4_CTI] = KONA_BMODEM_CTI_VA,
	.ETB_base = KONA_ETB_VA,
	.ETB2AXI_base = KONA_ETB2AXI_VA,
	.GLOBPERF_base = KONA_GLOBPERF_VA,
	.ATB_STM_base = KONA_STM_VA,
	.SW_STM_base[CHAL_TRACE_SWSTM] = KONA_SWSTM_VA,
	.SW_STM_base[CHAL_TRACE_SWSTM_ST] = KONA_SWSTM_ST_VA,
	.GICTR_base = KONA_GICTR_VA,
};
static CHAL_HANDLE kona_trace_handle;

static int kona_trace_funnel_set_enable(CHAL_TRACE_FUNNEL_t funnel_type,
					uint8_t port_n, int enable)
{
	int status;

	if (chal_trace_funnel_locked(kona_trace_handle, funnel_type)) {
		chal_trace_funnel_set_lock_access(kona_trace_handle,
						  funnel_type,
						  LOCK_ACCESS_CODE);
	}

	status = chal_trace_funnel_set_enable(kona_trace_handle,
					      funnel_type, port_n, enable);
	return status;
}

static void kona_trace_set_stm_en(uint8_t bit, int set)
{
	uint32_t reg_high, reg_low;

	reg_high = chal_trace_atb_stm_get_en(kona_trace_handle, 1);
	reg_low = chal_trace_atb_stm_get_en(kona_trace_handle, 0);

	if (bit < 32) {
		if (set)
			reg_low |= (1 << bit);
		else
			reg_low &= ~(1 << bit);

	} else if (bit < 64) {
		if (set)
			reg_high |= (1 << (bit - 32));
		else
			reg_high &= ~(1 << (bit - 32));

	}

	chal_trace_atb_stm_set_en(kona_trace_handle, reg_low, reg_high);
}

static void kona_trace_set_stm_sw(uint8_t bit, int set)
{
	uint32_t reg_high, reg_low;

	reg_high = chal_trace_atb_stm_get_sw(kona_trace_handle, 1);
	reg_low = chal_trace_atb_stm_get_sw(kona_trace_handle, 0);

	if (bit < 32) {
		if (set)
			reg_low |= (1 << bit);
		else
			reg_low &= ~(1 << bit);

	} else if (bit < 64) {
		if (set)
			reg_high |= (1 << (bit - 32));
		else
			reg_high &= ~(1 << (bit - 32));

	}

	chal_trace_atb_stm_set_sw(kona_trace_handle, reg_low, reg_high);
}

int kona_trace_set_sw_stm(int on)
{
	int status = 0;

	if (kona_trace_handle == NULL)
		return -EIO;
	if (on) {
		/* SW STM Config */
		chal_trace_sw_stm_set_config(kona_trace_handle,
					     CHAL_TRACE_SWSTM, 1,
					     ATB_ID_SW_STM_A9);

		/* Enable ARM Funnel port6 */
		kona_trace_funnel_set_enable(CHAL_TRACE_FUNNEL, 6, 1);

		/* Enable FINAL Funnel port 1 */
		kona_trace_funnel_set_enable(stm.pdata->final_funnel, 1, 1);

		/* Set Enable for SWSTM */
		kona_trace_set_stm_en(ATB_ID_SW_STM_A9, 1);

		/* Set SW for SWSTM */
		kona_trace_set_stm_sw(ATB_ID_SW_STM_A9, 1);
	} else {
		/* Disable ARM Funnel port6 */
		kona_trace_funnel_set_enable(CHAL_TRACE_FUNNEL, 6, 0);

		/* Set Disable for SWSTM */
		kona_trace_set_stm_en(ATB_ID_SW_STM_A9, 0);
	}

	return status;
}

static int kona_trace_init(void)
{
	int status = -EIO;
	void __iomem *base;

	kona_trace_handle = NULL;
	base = HW_IO_PHYS_TO_VIRT(PAD_CTRL_BASE_ADDR);

	if (kona_trace_handle)
		return 0;
	if (etm_on == 0) {
		chal_trace_init(&trace_base_addr);
		kona_trace_handle = &trace_base_addr;

		if (kona_trace_handle)
			status = 0;
		/* STM config */
		/* 4 bits wide PTI output, always break, ATBID 0x0B */
		chal_trace_atb_stm_set_config(kona_trace_handle, 0, 0, 1,
				      ATB_ID_ODD(ATB_ID_STM));
		/* Turn on the A9 SWSTM */
		kona_trace_set_sw_stm(1);
	}

	/* overwrite dt-blob for ETM/PTM use */
	if (etm_on == 1) {
		writel(PTI_OFF, base + PADCTRLREG_TRACECLK_OFFSET);
		writel(PTI_OFF, base + PADCTRLREG_TRACEDT00_OFFSET);
		writel(PTI_OFF, base + PADCTRLREG_TRACEDT01_OFFSET);
		writel(PTI_OFF, base + PADCTRLREG_TRACEDT02_OFFSET);
		writel(PTI_OFF, base + PADCTRLREG_TRACEDT03_OFFSET);
		writel(PTI_OFF, base + PADCTRLREG_TRACEDT07_OFFSET);
	}

	return status;
}

int kona_trace_stm_write(int ch, int ts, size_t len, uint8_t *data)
{
	CHAL_TRACE_SWSTM_t st;

	if (kona_trace_handle == NULL)
		return -EIO;

	if (!data)
		return -EINVAL;

	if (ts)
		st = CHAL_TRACE_SWSTM_ST;
	else
		st = CHAL_TRACE_SWSTM;

	/* Align start before writing 4 bytes at a time */
	while ((len > 0) && (((uint32_t) data) & (sizeof(uint32_t) - 1))) {
		chal_trace_sw_stm_write(kona_trace_handle, st, ch, 1, *data);
		data++;
		len--;
	}

	/* Write 4 bytes at a time while we can */
	while (len >= 4) {
		uint32_t tmp = 0;
		tmp = ((((uint32_t) data[0]) << 24) |
		       (((uint32_t) data[1]) << 16) |
		       (((uint32_t) data[2]) << 8) |
		       (((uint32_t) data[3]) << 0));
		chal_trace_sw_stm_write(kona_trace_handle, st, ch, 4, tmp);
		data += 4;
		len -= 4;
	}

	/* Finish remaining bytes */
	while (len > 0) {
		chal_trace_sw_stm_write(kona_trace_handle, st, ch, 1, *data);
		data++;
		len--;
	}

	return 0;
}

int pti_enable(int enable)
{
	if (pti_peri_clk) {
		if (enable)
			clk_enable(pti_peri_clk);
		else
			clk_disable(pti_peri_clk);
	}
	return 0;
}

void stm_trace_buffer_start(int channel)
{
	uint8_t trace_type = 0x72;	/* make it configurable */
	uint32_t pti_read;

	pti_read =
	    BRCM_READ_REG_FIELD(KONA_CHIPREG_VA, CHIPREG_PERIPH_SPARE_CONTROL1,
				PTI_CLK_IS_IDLE);
	if (pti_read == 1) {
		pti_enable(1);
	}
	kona_trace_stm_write(channel, FALSE, 1, &trace_type);
	/* leave it on when enabled by DFSD */
	pti_enable(pti_read);
}

void stm_trace_buffer_end(int channel)
{
	uint8_t termination = 0;
	uint32_t pti_read;

	pti_read =
	    BRCM_READ_REG_FIELD(KONA_CHIPREG_VA, CHIPREG_PERIPH_SPARE_CONTROL1,
				PTI_CLK_IS_IDLE);
	if (pti_read == 1) {
		pti_enable(1);
	}
	kona_trace_stm_write(channel, TRUE, 1, &termination);
	/* leave it on when enabled by DFSD */
	pti_enable(pti_read);
}

int stm_trace_buffer_data(int channel, const void *data_ptr, size_t length)
{
	uint32_t pti_read;
	pti_read =
	    BRCM_READ_REG_FIELD(KONA_CHIPREG_VA, CHIPREG_PERIPH_SPARE_CONTROL1,
				PTI_CLK_IS_IDLE);
	if (pti_read == 1) {
		pti_enable(1);
	}
	kona_trace_stm_write(channel, FALSE, length, (uint8_t *) data_ptr);
	/* leave it on when enabled by DFSD */
	pti_enable(pti_read);
	return length;
}

int stm_trace_send_bytes(int channel, const void *data_ptr, size_t length)
{
	uint8_t trace_type = 0x72;	/* make it configurable */
	uint8_t termination = 0;
	uint32_t pti_read;

	/* DFSD enables PTI clock for HW trace. Read here and if enabled by it,
	 * then do not disable the clock. */
	pti_read =
	    BRCM_READ_REG_FIELD(KONA_CHIPREG_VA, CHIPREG_PERIPH_SPARE_CONTROL1,
				PTI_CLK_IS_IDLE);
	if (pti_read == 1) {
		pti_enable(1);
	}

	/* send trace type to start STM message */
	kona_trace_stm_write(channel, FALSE, 1, &trace_type);
	kona_trace_stm_write(channel, FALSE, length, (uint8_t *) data_ptr);
	kona_trace_stm_write(channel, TRUE, 1, &termination);

	/* leave it on when enabled by DFSD */
	pti_enable(pti_read);
	return length;
}

void check_pti_disable(void)
{
	int pti_stat;

	pti_stat =
	    BRCM_READ_REG_FIELD(KONA_CHIPREG_VA, CHIPREG_PERIPH_SPARE_CONTROL1,
				PTI_CLK_IS_IDLE);
	if (!pti_stat)
		pti_enable(0);
}
#endif

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

static int stm_get_channel(struct channel_data *ch_data, int __user * arg)
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
int stm_trace_buffer_onchannel(int channel, const void *data, size_t length)
{
#ifdef CONFIG_BCM_STM
	stm_trace_send_bytes(channel, data, length);
#endif
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
	channel_data->numero = -1;	/*  Channel not yet allocated */
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

static ssize_t stm_write(struct file *file, const char __user * buf,
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

	if (copy_from_user(channel->data_buffer, (void __user *)buf, size)) {
		spin_unlock(&channel->lock);
		return -EFAULT;
	}
	size = stm_trace_buffer_onchannel(channel->numero,
					  channel->data_buffer, size);

	spin_unlock(&channel->lock);

	return size;
}

/* Enable the trace for given sources (bitfield) */
static void stm_enable_src(unsigned int v)
{
}

/* Disable all sources */
static void stm_disable_src(void)
{
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
	.owner = THIS_MODULE,
	.unlocked_ioctl = stm_ioctl,
	.open = stm_open,
	.llseek = no_llseek,
	.write = stm_write,
	.release = stm_release,
};

/*
 * Init and deinit driver
 */

static int stm_probe(struct platform_device *pdev)
{
	int retval = 0;

	if (!pdev || !pdev->dev.platform_data) {
		pr_alert("No device/platform_data found on STM driver\n");
		return -ENODEV;
	}

	stm.pdata = pdev->dev.platform_data;

	pti_peri_clk = clk_get(NULL, PTI_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(pti_peri_clk))
		return -EIO;

	cdev_init(&cdev, &stm_fops);
	cdev.owner = THIS_MODULE;

	retval = cdev_add(&cdev, MKDEV(stm_major, 0), 1);
	if (retval) {
		dev_err(&pdev->dev, "chardev registration failed\n");
		goto err_channels;
	}

	if (IS_ERR(device_create(stm_class, &pdev->dev,
				 MKDEV(stm_major, 0), NULL, STM_DEV_NAME)))
		dev_err(&pdev->dev, "can't create device\n");

	/* Reserve channels if necessary */
	if (stm.pdata->channels_reserved_sz) {
		int i;

		for (i = 0; i < stm.pdata->channels_reserved_sz; i++)
			set_bit(stm.pdata->channels_reserved[i], stm.ch_bitmap);

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

	/* Enable STM Masters given in pdata */
	if (stm.pdata->masters_enabled)
		stm_enable_src(stm.pdata->masters_enabled);

#ifdef CONFIG_BCM_STM
	kona_trace_init();
#endif

	dev_info(&pdev->dev, "STM-Trace driver probed successfully\n");
	pr_info("STM-Trace driver initialized\n");
	return 0;

err_channels:
	return retval;
}

static int stm_remove(struct platform_device *pdev)
{
	device_destroy(stm_class, MKDEV(stm_major, 0));
	cdev_del(&cdev);

	stm_disable_src();

	return 0;
}

int stm_printk(const char *fmt, ...)
{
	int ret;
	size_t size;
	va_list args;
	unsigned long flags;

	va_start(args, fmt);
	spin_lock_irqsave(&stm_buf_lock, flags);
	size = vscnprintf((char *)stm_printk_buf,
			  sizeof(stm_printk_buf), fmt, args);
	ret = stm_trace_buffer(stm_printk_buf, size);
	spin_unlock_irqrestore(&stm_buf_lock, flags);
	va_end(args);
	return ret;
}
EXPORT_SYMBOL(stm_printk);

/*
 * Debugfs interface
 */

/* Count # of free channels */
static int stm_free_channels_show(void *data, u64 * val)
{
	*val = stm_nb_free_channels();
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(stm_free_channels_fops, stm_free_channels_show,
			NULL, "%lld\n");

static __init int stm_init_debugfs(void)
{
	struct dentry *d_stm;

	d_stm = debugfs_create_dir(STM_DEV_NAME, NULL);
	if (!d_stm)
		return -ENOMEM;

	(void)debugfs_create_file("free_channels", S_IRUGO, d_stm,
				  NULL, &stm_free_channels_fops);
	return 0;
}

fs_initcall(stm_init_debugfs);

static struct platform_driver stm_driver = {
	.probe = stm_probe,
	.remove = stm_remove,
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

arch_initcall(stm_init);	/* STM init ASAP need to wait GPIO init */
module_exit(stm_exit);

MODULE_AUTHOR("Paul Ghaleb - ST Microelectronics");
MODULE_AUTHOR("Pierre Peiffer - ST-Ericsson");
MODULE_AUTHOR("Philippe Langlais - ST-Ericsson");
MODULE_DESCRIPTION("System Trace Module driver");
MODULE_ALIAS("stm");
MODULE_ALIAS("stm-trace");
MODULE_LICENSE("GPL v2");
