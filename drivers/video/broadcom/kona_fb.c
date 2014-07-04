/****************************************************************************
*
*	Copyright (c) 1999-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*   at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a
*   license other than the GPL, without Broadcom's express prior written
*   consent.
*
****************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/acct.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/ipc/ipc.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/vt_kern.h>
#include <linux/gpio.h>
#include <video/kona_fb.h>
#include <mach/io.h>
#include <linux/delay.h>
#include <mach/memory.h>
#include <mach/io_map.h>
#include <plat/reg_axipv.h>
#include <asm/io.h>
#ifdef CONFIG_FRAMEBUFFER_FPS
#include <linux/fb_fps.h>
#endif
#include <linux/clk.h>
#include <plat/pi_mgr.h>
#include <linux/broadcom/mobcom_types.h>
#include <linux/reboot.h>
#include <linux/kdebug.h>
#include <linux/notifier.h>
#include "kona_fb.h"
#include "lcd/display_drv.h"
#include "lcd/lcd.h"

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/kobject.h>
#include <linux/seq_file.h>
#include <linux/ctype.h>
#endif /*  CONFIG_DEBUG_FS */

#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
#include <video/kona_fb_image_dump.h>
#include "lcd/dump_start_img.h"
#include "lcd/dump_end_img.h"
#include "lcd/ap_start_dump_img.h"
#include "lcd/cp_start_dump_img.h"
#include "lcd/ap_ramdump_start_img.h"
#include "lcd/cp_ramdump_start_img.h"
#endif
#ifdef CONFIG_IOMMU_API
#include <linux/iommu.h>
#include <plat/bcm_iommu.h>
#endif

#ifdef CONFIG_DEBUG_FS
#define DBGFS_MAX_BUFF_SIZE 200
#endif /* CONFIG_DEBUG_FS */

/*#define KONA_FB_DEBUG */
/*#define PARTIAL_UPDATE_SUPPORT */

#define KONA_IOCTL_SET_BUFFER_AND_UPDATE	_IO('F', 0x80)
#define KONA_IOCTL_GET_FB_IOVA			_IOR('F', 0x81, u32)

static struct pi_mgr_qos_node g_mm_qos_node;

#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
static DEFINE_SPINLOCK(g_fb_crash_spin_lock);
#endif

enum lcd_state {
	KONA_FB_UNBLANK,
	KONA_FB_BLANK,
	KONA_FB_LINK_SUSPENDED,
};

enum link_ctrl {
	RESUME_LINK,
	SUSPEND_LINK,
};

struct kona_fb {
	spinlock_t lock;
	struct mutex update_sem;
	struct completion prev_buf_done_sem;
	atomic_t buff_idx;
	atomic_t is_fb_registered;
	atomic_t is_graphics_started;
	int rotate;
	int is_display_found;
#ifdef CONFIG_FRAMEBUFFER_FPS
	struct fb_fps_info *fps_info;
#endif
	struct fb_info fb;
	u32 cmap[16];
	DISPDRV_T *display_ops;
	DISPDRV_INFO_T *display_info;
	DISPDRV_HANDLE_T display_hdl;
	struct pi_mgr_dfs_node dfs_node;
	int g_stop_drawing;
	struct proc_dir_entry *proc_entry;
	enum lcd_state blank_state;
	void *buff0;
	void *buff1;
	atomic_t force_update;
	struct notifier_block reboot_nb;
#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
	struct notifier_block die_nb;
#endif
	struct work_struct vsync_smart;

	size_t framesize_alloc;
	struct iommu_domain *direct_domain;
	struct sg_table *iovmm_table;
	struct kona_fb_platform_data *fb_data;
	struct platform_device *pdev;

	/* ESD check routine */
	struct workqueue_struct *esd_check_wq;
	struct delayed_work esd_check_work;
	struct completion tectl_gpio_done_sem;
	int esd_failure_cnt;
	/* user count */
	unsigned short open_count;

#ifdef CONFIG_DEBUG_FS
	struct dentry *dbgfs_dir;
#endif
};

static struct completion vsync_event;
static struct kona_fb *g_kona_fb;

#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
static void kona_fb_unpack_888rle(void *, void *, uint32_t, uint32_t, uint32_t);
static void kona_fb_unpack_565rle(void *, void *, uint32_t, uint32_t, uint32_t);
#endif

static int kona_fb_reboot_cb(struct notifier_block *, unsigned long, void *);
#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
static int kona_fb_die_cb(struct notifier_block *, unsigned long, void *);
#endif

static int lcd_boot_mode(char *);
static int lcd_panel_setup(char *);
static int kona_fb_blank(int blank_mode, struct fb_info *info);

static int need_page_alignment = 1;
static char g_disp_str[DISPDRV_NAME_SZ];
int g_display_enabled;

#ifdef KONA_FB_DEBUG
#define KONA_PROF_N_RECORDS 50
static volatile struct {
	struct timeval	curr_time;
	struct timeval	prev_time;
	int is_late;
	int use_te;
} kona_fb_profile[KONA_PROF_N_RECORDS];

static volatile u32 kona_fb_profile_cnt;

void kona_fb_profile_record(struct timeval prev_timeval,
		struct timeval curr_timeval, int is_too_late, int do_vsync)
{
	kona_fb_profile[kona_fb_profile_cnt].curr_time =  curr_timeval;
	kona_fb_profile[kona_fb_profile_cnt].prev_time =  prev_timeval;
	kona_fb_profile[kona_fb_profile_cnt].is_late   = is_too_late;
	kona_fb_profile[kona_fb_profile_cnt].use_te   = do_vsync;
	kona_fb_profile_cnt = (kona_fb_profile_cnt+1) % KONA_PROF_N_RECORDS;
}

static int
proc_write_fb_test(struct file *file, const char __user *buffer,
			size_t count, loff_t *data)
{
	int len, cnt, i, num;
	char value[20];

	if (count > 19)
		len = 19;
	else
		len = count;

	if (copy_from_user(value, buffer, len))
		return -EFAULT;

	value[len] = '\0';

	num = simple_strtoul(value, NULL, 0);

	if (num == 1)
		kona_display_crash_image(0);
	else
		kona_display_crash_image(1);

	i = kona_fb_profile_cnt;
	for (cnt = 0; cnt < KONA_PROF_N_RECORDS; cnt++) {
		if (i != 0)
			printk(KERN_ERR "%8u,%8u,%8u,%8u,%8u,%d, %d",
			kona_fb_profile[i].prev_time.tv_sec,
			kona_fb_profile[i].prev_time.tv_usec,
			kona_fb_profile[i].curr_time.tv_sec,
			kona_fb_profile[i].curr_time.tv_usec,
			(kona_fb_profile[i].prev_time.tv_sec -
			 kona_fb_profile[i-1].curr_time.tv_sec) * 1000000 +
			(kona_fb_profile[i].prev_time.tv_usec -
			 kona_fb_profile[i-1].curr_time.tv_usec),
			kona_fb_profile[i].is_late,
			kona_fb_profile[i].use_te);
		i = (i + 1) % KONA_PROF_N_RECORDS;
	}

	return len;
}
#else
#define kona_fb_profile_record(prev_timeval, curr_timeval, is_too_late, \
		do_vsync) do { } while (0)
#define proc_write_fb_test NULL
#endif /* KONA_FB_DEBUG */

static inline u32 convert_bitfield(int val, struct fb_bitfield *bf)
{
	unsigned int mask = (1 << bf->length) - 1;

	return (val >> (16 - bf->length) & mask) << bf->offset;
}

static int
kona_fb_setcolreg(unsigned int regno, unsigned int red, unsigned int green,
		  unsigned int blue, unsigned int transp, struct fb_info *info)
{
	struct kona_fb *fb = container_of(info, struct kona_fb, fb);

	konafb_debug("kona regno = %d r=%d g=%d b=%d\n", regno, red, green,
		     blue);

	if (regno < 16) {
		fb->cmap[regno] = convert_bitfield(transp, &fb->fb.var.transp) |
		    convert_bitfield(blue, &fb->fb.var.blue) |
		    convert_bitfield(green, &fb->fb.var.green) |
		    convert_bitfield(red, &fb->fb.var.red);
		return 0;
	} else {
		return 1;
	}
}

static int kona_fb_check_var(struct fb_var_screeninfo *var,
			     struct fb_info *info)
{
	konafb_debug("kona %s\n", __func__);

	if ((var->rotate & 1) != (info->var.rotate & 1)) {
		if ((var->xres != info->var.yres) ||
		    (var->yres != info->var.xres) ||
		    (var->xres_virtual != info->var.yres) ||
		    (var->yres_virtual > info->var.xres * 2) ||
		    (var->yres_virtual < info->var.xres)) {
			konafb_error("fb_check_var_failed\n");
			return -EINVAL;
		}
	} else {
		if ((var->xres != info->var.xres) ||
		    (var->yres != info->var.yres) ||
		    (var->xres_virtual != info->var.xres) ||
		    (var->yres_virtual > info->var.yres * 2) ||
		    (var->yres_virtual < info->var.yres)) {
			konafb_error("fb_check_var_failed\n");
			return -EINVAL;
		}
	}

	return 0;
}

static int kona_fb_set_par(struct fb_info *info)
{
#if 0
	struct kona_fb *fb = container_of(info, struct kona_fb, fb);

	konafb_debug("kona %s\n", __func__);

	if (fb->rotation != fb->fb.var.rotate) {
		konafb_warning("Rotation is not supported yet !\n");
		return -EINVAL;
	}
#endif

	return 0;
}

static inline void kona_clock_start(struct kona_fb *fb)
{
	fb->display_ops->start(fb->display_hdl, &fb->dfs_node);
}

static inline void kona_clock_stop(struct kona_fb *fb)
{
	fb->display_ops->stop(fb->display_hdl, &fb->dfs_node);
}

static void link_control(struct kona_fb *fb, enum link_ctrl link_ctrl)
{
	if (link_ctrl == RESUME_LINK) {
		pi_mgr_qos_request_update(&g_mm_qos_node, 10);
		if (!fb->display_info->vmode)
			kona_clock_start(fb);
		fb->display_ops->resume_link(fb->display_hdl);
		if (!fb->display_info->vmode)
			kona_clock_stop(fb);
	} else {
		if (!fb->display_info->vmode)
			kona_clock_start(fb);
		fb->display_ops->suspend_link(fb->display_hdl);
		if (!fb->display_info->vmode)
			kona_clock_stop(fb);
		pi_mgr_qos_request_update(&g_mm_qos_node,
						PI_MGR_QOS_DEFAULT_VALUE);
	}
}

#ifdef CONFIG_IOMMU_API
static int kona_fb_direct_map(struct kona_fb *fb, size_t framesize_alloc,
			dma_addr_t phys_fbbase, dma_addr_t dma_addr)
{
	struct iommu_domain *domain = NULL;
	int ret = -ENXIO;
	struct kona_fb_platform_data *fb_data = fb->fb_data;
	struct platform_device *pdev = fb->pdev;

	/* 1-to-1 mapping */
	domain = iommu_domain_alloc(&platform_bus_type);
	if (NULL == domain)
		goto fail_alloc;

	if (iommu_attach_device(domain, &pdev->dev)) {
		ret = -EINVAL;
		pr_err("%s Attaching dev(%p) to iommu dev(%p) failed\n",
				"framebuffer", &pdev->dev,
				&fb_data->pdev_iommu->dev);
		goto fail_attach;
	}

	if (iommu_map(domain, dma_addr, phys_fbbase, framesize_alloc,
				0)) {
		ret = -EINVAL;
		pr_err("%s iommu mapping domain(%p) da(%#x) to pa(%#x) size(%#08x) failed\n",
				"framebuffer", domain, dma_addr,
				phys_fbbase, framesize_alloc);
		goto fail_map;
	}

	fb->direct_domain = domain;
	pr_info("%s succ\n", __func__);
	return 0;

fail_map:
	iommu_detach_device(domain, &pdev->dev);
fail_attach:
	iommu_domain_free(domain);
fail_alloc:
	pr_err("%s failed ret = %d\n", __func__, ret);
	return ret;
}

static int kona_fb_direct_unmap(struct kona_fb *fb,
			size_t framesize_alloc, dma_addr_t dma_addr)
{
	struct iommu_domain *domain = fb->direct_domain;
	struct platform_device *pdev = fb->pdev;

	if (NULL == domain) {
		pr_err("%s fail, direct_domain NULL\n", __func__);
		return -EIO;
	}

	iommu_unmap(domain, dma_addr, framesize_alloc);

	iommu_detach_device(domain, &pdev->dev);

	iommu_domain_free(domain);

	fb->direct_domain = NULL;
	pr_info("%s succ\n", __func__);
	return 0;
}

#ifdef CONFIG_BCM_IOVMM
static int kona_fb_iovmm_map(struct kona_fb *fb, size_t framesize_alloc,
			dma_addr_t phys_fbbase, dma_addr_t *p_dma_addr)
{
	int n_pages, i;
	struct scatterlist *sg;
	struct sg_table *table;
	int ret = -ENXIO;
	struct dma_iommu_mapping *mapping = NULL;
	struct kona_fb_platform_data *fb_data = fb->fb_data;
	struct platform_device *pdev = fb->pdev;
	dma_addr_t dma_addr;

	if (!fb_data->pdev_iovmm) {
		ret = -EINVAL;
		pr_err("%s: iovmm device not set\n", "framebuffer");
		goto fail;
	}
	mapping = platform_get_drvdata(fb_data->pdev_iovmm);
	arm_iommu_attach_device(&pdev->dev, mapping);
	pr_info("%s iommu-mapping(%p)\n", "framebuffer", mapping);

	table = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!table) {
		ret = -ENOMEM;
		pr_err("Unable to allocate fb sgtable\n");
		goto fail;
	}
	n_pages = framesize_alloc >> PAGE_SHIFT;

	i = sg_alloc_table(table, 1, GFP_KERNEL);
	if (i) {
		ret = -ENOMEM;
		pr_err("sg_alloc_table failed\n");
		goto fail_sg_alloc;
	}
	for_each_sg(table->sgl, sg, table->nents, i) {
		struct page *page =
			phys_to_page(phys_fbbase);
		sg_set_page(sg, page, PAGE_SIZE * n_pages, 0);
	}
	dma_addr = arm_iommu_map_sgt(&pdev->dev, table, 0);
	if (dma_addr == DMA_ERROR_CODE) {
		ret = -EINVAL;
		pr_err("%16s: Failed iommu map da(%#x) pa(%#x) size(%#x)\n",
				"framebuffer", dma_addr, phys_fbbase,
				framesize_alloc);
		goto fail_map_sgt;
	}

	fb->iovmm_table = table;
	*p_dma_addr = dma_addr;
	pr_info("%s succ\n", __func__);
	return 0;

fail_map_sgt:
	sg_free_table(table);
fail_sg_alloc:
	kfree(table);
fail:
	pr_err("%s failed ret = %d\n", __func__, ret);
	return ret;
}

static int kona_fb_iovmm_unmap(struct kona_fb *fb,
			size_t framesize_alloc, dma_addr_t dma_addr)
{
	struct platform_device *pdev = fb->pdev;

	arm_iommu_unmap(&pdev->dev, dma_addr, framesize_alloc);

	if (fb->iovmm_table) {
		sg_free_table(fb->iovmm_table);
		kfree(fb->iovmm_table);
		fb->iovmm_table = NULL;
	}

	pr_info("%s succ\n", __func__);
	return 0;
}
#endif
#endif

#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
static void kona_fb_unpack_565rle(void *dst, void *src, uint32_t image_size,
				uint32_t img_w, uint32_t img_h)
{
	unsigned long count, index, len, data, pos;
	u16 *lcd_buf, *image_buf;

	image_buf = (u16 *)src;
	lcd_buf = (u16 *)dst;
	for (count = 0, pos = 0; count < image_size / 2; count += 2) {
		len = image_buf[count];
		data = image_buf[count + 1];
		for (index = 0; index < len; index++) {
			lcd_buf[pos++] = data;
			if (pos >  g_kona_fb->fb.fix.smem_len / 4)
				printk(KERN_ERR "Wrong image size!");
		}
	}
}

static void kona_fb_unpack_888rle(void *dst, void *src, uint32_t image_size,
				uint32_t img_w, uint32_t img_h)
{
	unsigned long count, len, data, pos, pix_left_curr_line;
	unsigned long x_margin, y_margin;
	u32 *lcd_buf;
	u16 *image_buf;
	int dir;

	struct fb_info *fb = &g_kona_fb->fb;

	image_buf = (u16 *)src;
	lcd_buf = (u32 *)dst;
	x_margin = (fb->var.xres - img_w) / 2;
	y_margin = (fb->var.yres - img_h) / 2;

	/*If rotation is enabled, move to the end of buffer*/
	if (g_kona_fb->rotate == FB_ROTATE_UD) {
		pos = (fb->var.xres * fb->var.yres - 1);
		dir = -1;
	} else {
		pos = 0;
		dir = 1;
	}
	pix_left_curr_line = img_w;
	pos += y_margin * fb->var.xres * dir;
	pos += x_margin * dir;
	for (count = 0; count < image_size; count += 8) {
		len = *image_buf++;
		len |= (*image_buf++) << 16;
		data = *image_buf++;
		data |= (*image_buf++ << 16);
		while (len) {
			while (pix_left_curr_line) {
				lcd_buf[pos] = data;
				pos += dir;
				--len;
				--pix_left_curr_line;
				if (pos > fb->fix.smem_len / 8)
					printk(KERN_ERR "Wrong image size!");
				if (!len)
					break;
			}
			if (!pix_left_curr_line) {
				pos += x_margin * 2 * dir;
				pix_left_curr_line = img_w;
			}
		}
	}
}

void kona_display_crash_image(enum crash_dump_image_idx image_idx)
{
	unsigned long flags, image_size, img_w, img_h;
	void *image_buf;
	static bool crash_displayed;

	if (panic_timeout == 1)
		return;

	pr_err("%s:%d image_idx=%d\n", __func__, __LINE__, image_idx);
	atomic_set(&g_kona_fb->force_update, 1);
	kona_clock_start(g_kona_fb);
	spin_lock_irqsave(&g_fb_crash_spin_lock, flags);

	switch (image_idx) {
	case GENERIC_DUMP_START:
		if (16 == g_kona_fb->fb.var.bits_per_pixel) {
			image_buf = (void *) &dump_start_img_565[0];
			image_size = sizeof(dump_start_img_565);
		} else {
			image_buf = (void *) &dump_start_img_888[0];
			image_size = sizeof(dump_start_img_888);
		}
		img_w = dump_start_img_w;
		img_h = dump_start_img_h;
		break;
	case CP_CRASH_DUMP_START:
		if (16 == g_kona_fb->fb.var.bits_per_pixel) {
			image_buf = (void *) &cp_dump_start_img_565[0];
			image_size = sizeof(cp_dump_start_img_565);
		} else {
			image_buf = (void *) &cp_dump_start_img_888[0];
			image_size = sizeof(cp_dump_start_img_888);
		}
		img_w = dump_cp_start_img_w;
		img_h = dump_cp_start_img_h;
		break;
	case AP_CRASH_DUMP_START:
		if (16 == g_kona_fb->fb.var.bits_per_pixel) {
			image_buf = (void *) &ap_dump_start_img_565[0];
			image_size = sizeof(ap_dump_start_img_565);
		} else {
			image_buf = (void *) &ap_dump_start_img_888[0];
			image_size = sizeof(ap_dump_start_img_888);
		}
		img_w = dump_ap_start_img_w;
		img_h = dump_ap_start_img_h;
		break;
	case GENERIC_DUMP_END:
	case CP_CRASH_DUMP_END:
	case AP_CRASH_DUMP_END:
		if (16 == g_kona_fb->fb.var.bits_per_pixel) {
			image_buf = (void *) &dump_end_img_565[0];
			image_size = sizeof(dump_end_img_565);
		} else {
			image_buf = (void *) &dump_end_img_888[0];
			image_size = sizeof(dump_end_img_888);
		}
		img_w = dump_end_img_w;
		img_h = dump_end_img_h;
		break;
	case CP_RAM_DUMP_START:
		if (16 == g_kona_fb->fb.var.bits_per_pixel) {
			image_buf = (void *) &cp_ramdump_start_img_565[0];
			image_size = sizeof(cp_ramdump_start_img_565);
		} else {
			image_buf = (void *) &cp_ramdump_start_img_888[0];
			image_size = sizeof(cp_ramdump_start_img_888);
		}
		img_w = cp_ramdump_start_img_w;
		img_h = cp_ramdump_start_img_h;
		break;
	case AP_RAM_DUMP_START:
		if (16 == g_kona_fb->fb.var.bits_per_pixel) {
			image_buf = (void *) &ap_ramdump_start_img_565[0];
			image_size = sizeof(ap_ramdump_start_img_565);
		} else {
			image_buf = (void *) &ap_ramdump_start_img_888[0];
			image_size = sizeof(ap_ramdump_start_img_888);
		}
		img_w = ap_ramdump_start_img_w;
		img_h = ap_ramdump_start_img_h;
		break;
	default:
		pr_err("Invalid image index passed\n");
		goto err_idx;
	}

	if (16 == g_kona_fb->fb.var.bits_per_pixel)
		kona_fb_unpack_565rle((void *)g_kona_fb->fb.screen_base,
					image_buf, image_size, img_w, img_h);
	else
		kona_fb_unpack_888rle((void *)g_kona_fb->fb.screen_base,
					image_buf, image_size, img_w, img_h);

	/* For video mode, it is sufficient if we draw on the active buffer*/
	if (!g_kona_fb->display_info->vmode || (false == crash_displayed)) {
		if (g_kona_fb->display_ops->update_no_os)
			g_kona_fb->display_ops->update_no_os(
				g_kona_fb->display_hdl, g_kona_fb->buff0, NULL);
	}

	crash_displayed = true;
err_idx:
	g_kona_fb->g_stop_drawing = 1;
	spin_unlock_irqrestore(&g_fb_crash_spin_lock, flags);
	kona_clock_stop(g_kona_fb);
}
#endif

static void kona_display_done_cb(int status)
{
	(void)status;
	if (!g_kona_fb->display_info->vmode)
		kona_clock_stop(g_kona_fb);
	konafb_debug("kona_fb release called\n");
	complete(&g_kona_fb->prev_buf_done_sem);
}

/* This function peforms the in-place rotation of the buffer */
int kona_fb_rotate_buffer(void *buffer, int deg, unsigned width,
				unsigned height, unsigned char bytes_per_pixel)
{
	int i = 0;
	unsigned int total_num_pixels = width * height;
	unsigned short *Bottom_2Bpp;
	unsigned short *Top_2Bpp;
	unsigned short temp_2Bpp;
	unsigned int *Bottom_4Bpp;
	unsigned int *Top_4Bpp;
	unsigned int temp_4Bpp;

	if (bytes_per_pixel == 2) {
		/* point to the first pixel */
		Top_2Bpp = buffer;
		/* point to the last pixel */
		Bottom_2Bpp = Top_2Bpp + (width * height) - 1;

		for (i = 0; i < total_num_pixels / 2; ++i) {
			/* swap pixels from top-left in forward direction
			   to pixels in bottom-right in reverse direction */
			temp_2Bpp = *Top_2Bpp;
			*Top_2Bpp++ = *Bottom_2Bpp;
			*Bottom_2Bpp-- = temp_2Bpp;
		}
	} else {
	/* if Bpp = 4 */
		/* point to the first pixel */
		Top_4Bpp = buffer;
		/* point to the last pixel */
		Bottom_4Bpp = Top_4Bpp + (width * height) - 1;

		for (i = 0; i < total_num_pixels / 2; ++i) {
			/* swap pixels from top-left in forward direction
			   to pixels in bottom-right in reverse direction */
			temp_4Bpp = *Top_4Bpp;
			*Top_4Bpp++ = *Bottom_4Bpp;
			*Bottom_4Bpp-- = temp_4Bpp;
		}
	}
	return 0;
}

static int kona_fb_pan_display(struct fb_var_screeninfo *var,
			       struct fb_info *info)
{
	int ret = 0;
	struct kona_fb *fb = container_of(info, struct kona_fb, fb);
	uint32_t buff_idx;
	int bytes_per_pixel = 0;
#ifdef CONFIG_FRAMEBUFFER_FPS
	void *dst;
#endif
	DISPDRV_WIN_t region, *p_region;

	buff_idx = var->yoffset ? 1 : 0;

	if (var->rotate == FB_ROTATE_UD) {
		bytes_per_pixel = var->bits_per_pixel / 8;
		konafb_debug("Rotating inside kernel\n");

		if (buff_idx == 1) {
			if (kona_fb_rotate_buffer(fb->fb.screen_base +
				(fb->buff1 - fb->buff0), FB_ROTATE_UD ,
				var->xres, var->yres, bytes_per_pixel))
				konafb_error("Unable to rotate !!\n");
		} else {
			if (kona_fb_rotate_buffer(fb->fb.screen_base,
				FB_ROTATE_UD, var->xres, var->yres,
							bytes_per_pixel))
				konafb_error("Unable to rotate !!\n");
		}
	}
	konafb_debug("kona %s with buff_idx =%d\n", __func__, buff_idx);

	if (mutex_lock_killable(&fb->update_sem))
		return -EINTR;

	if (1 == fb->g_stop_drawing) {
		konafb_debug(
		"kona FB/LCd is in the early suspend state and stops drawing now!");
		goto skip_drawing;
	}

	atomic_set(&fb->buff_idx, buff_idx);

#ifdef CONFIG_FRAMEBUFFER_FPS
	dst = (fb->fb.screen_base) +
	    (buff_idx * fb->fb.var.xres * fb->fb.var.yres *
	     (fb->fb.var.bits_per_pixel / 8));
	fb_fps_display(fb->fps_info, dst, 5, 2, 0);
#endif

	if (fb->blank_state == KONA_FB_LINK_SUSPENDED) {
		konafb_debug("Resume link\n");
		link_control(fb, RESUME_LINK);
	}

	if (!atomic_read(&fb->is_fb_registered)) {
		if (!fb->display_info->vmode)
			kona_clock_start(fb);
		ret =
		    fb->display_ops->update(fb->display_hdl,
					    buff_idx ? fb->buff1 : fb->buff0,
					    NULL, NULL);
		if (!fb->display_info->vmode)
			kona_clock_stop(fb);
	} else {
		atomic_set(&fb->is_graphics_started, 1);
		if (var->reserved[0] == 0x54445055) {
			region.t = var->reserved[1] >> 16;
			region.l = (u16) var->reserved[1];
			region.b = (var->reserved[2] >> 16) - 1;
			region.r = (u16) var->reserved[2] - 1;
			region.w = region.r - region.l + 1;
			region.h = region.b - region.t + 1;
			region.mode = 0;
			p_region = &region;
		} else {
			region.t	= 0;
			region.l	= 0;
			region.b	= fb->fb.var.height - 1;
			region.r	= fb->fb.var.width - 1;
			region.w	= fb->fb.var.width;
			region.h	= fb->fb.var.height;
			region.mode	= 1;
			p_region = NULL;
		}
		if (!fb->display_info->vmode) {
			if (wait_for_completion_timeout(
			&fb->prev_buf_done_sem,	msecs_to_jiffies(10000)) <= 0)
				pr_err("%s:%d timed out waiting for completion",
					__func__, __LINE__);
			kona_clock_start(fb);
		}
		ret =
		    fb->display_ops->update(fb->display_hdl,
					buff_idx ? fb->buff1 : fb->buff0,
					p_region,
					(DISPDRV_CB_T)kona_display_done_cb);

		if (fb->display_info->vmode) {
			konafb_debug("waiting for release of 0x%x\n",
					buff_idx ? fb->buff0 : fb->buff1);
			if (wait_for_completion_timeout(
			&fb->prev_buf_done_sem,	msecs_to_jiffies(10000)) <= 0)
				pr_err("%s:%d timed out waiting for completion",
					__func__, __LINE__);
		}
	}

	if (fb->blank_state == KONA_FB_LINK_SUSPENDED) {
		konafb_debug("Suspend link again\n");
		link_control(fb, SUSPEND_LINK);
	}
skip_drawing:
	mutex_unlock(&fb->update_sem);

	return ret;
}

#if defined(CONFIG_MACH_BCM_FPGA_E) || defined(CONFIG_MACH_BCM_FPGA)
static int kona_fb_sync(struct fb_info *info)
{
	pr_info("[KONA_FB]: HW Composer not enabled on Java Eve, return\n");
	return 0;
}
#else
static int kona_fb_sync(struct fb_info *info)
{
	wait_for_completion_interruptible(&vsync_event);
	return 0;
}
#endif
static void konafb_vsync_cb(void)
{
	if (g_kona_fb && g_kona_fb->display_info->vmode)
		complete(&vsync_event);
}

static void vsync_work_smart(struct work_struct *work)
{
	struct kona_fb *fb = container_of(work, struct kona_fb,
						vsync_smart);

	complete(&vsync_event);
	/* 16ms ~ 60HZ */
	usleep_range(16000, 16010);
	schedule_work(&fb->vsync_smart);
}

static ssize_t kona_fb_panel_name_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct kona_fb *fb = dev_get_drvdata(dev);
	char const *name =  fb->fb_data->name ?
			fb->fb_data->name : "NoName";
	return scnprintf(buf, PAGE_SIZE, "Panel: %s\n", name);
}

static ssize_t kona_fb_panel_id_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct kona_fb *fb = dev_get_drvdata(dev);
	uint8_t res_da = 0;
	uint8_t res_db = 0;
	uint8_t res_dc = 0;

	if (!fb->display_info->vmode)
		kona_clock_start(fb);
	panel_read(0xDA, &res_da, 1);
	panel_read(0xDB, &res_db, 1);
	panel_read(0xDC, &res_dc, 1);
	if (!fb->display_info->vmode)
		kona_clock_stop(fb);

	return scnprintf(buf, PAGE_SIZE, "Panel ID: 0x%.2x 0x%.2x 0x%.2x\n",
							res_da, res_db, res_dc);
}

static ssize_t kona_fb_panel_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct kona_fb *fb = dev_get_drvdata(dev);
	if (fb->display_info->special_mode_panel)
		return scnprintf(buf, PAGE_SIZE, "%d\n",
					fb->display_info->special_mode_on);
	else
		return scnprintf(buf, PAGE_SIZE, "%s\n", "Not supported");
}

static ssize_t kona_fb_panel_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = count;
	struct kona_fb *fb = dev_get_drvdata(dev);
	uint32_t val;

	if (fb->display_info->vmode) {
		konafb_error("Only command mode supported\n");
		ret = -EOPNOTSUPP;
		goto exit;
	}

	if (!fb->display_info->special_mode_panel) {
		konafb_error("Not supported\n");
		ret = -EOPNOTSUPP;
		goto exit;
	}

	if (sscanf(buf, "%i", &val) != 1) {
		konafb_error("Error, buf = %s\n", buf);
		ret = -EINVAL;
		goto exit;
	}

	konafb_info("panel mode %i\n", val);
	mutex_lock(&fb->update_sem);
	if (val) {
		/* Enter special mode */
		if (fb->blank_state != KONA_FB_UNBLANK) {
			konafb_error("Not in UNBLANK state (%d)\n",
							fb->blank_state);
			ret = -EINVAL;
			goto exit_unlock;
		}
		if (fb->fb_data->esdcheck)
			cancel_delayed_work(&fb->esd_check_work);
		cancel_work_sync(&fb->vsync_smart);

		if (wait_for_completion_timeout(&fb->prev_buf_done_sem,
						msecs_to_jiffies(10000)) <= 0)
			konafb_error("timed out waiting for completion\n");
		kona_clock_start(fb);
		if (fb->display_info->cabc_enabled)
			panel_write(fb->display_info->cabc_off_seq);
		panel_write(fb->display_info->special_mode_on_seq);
		kona_clock_stop(fb);
		link_control(fb, SUSPEND_LINK);
		complete(&g_kona_fb->prev_buf_done_sem);
		fb->display_info->special_mode_on = true;
		fb->blank_state = KONA_FB_LINK_SUSPENDED;
		konafb_debug("Special mode ON\n");
	} else {
		/* Exit special mode */
		if (fb->blank_state == KONA_FB_BLANK) {
			/* If powerHAL exits special mode before system has
			   has sent KONA_FB_UNBLANK we will end up here. In
			   this state the display is already powered off */
			konafb_debug("Exit black mode\n");
			fb->display_info->special_mode_on = false;
			goto exit_unlock;
		}
		if (fb->blank_state != KONA_FB_LINK_SUSPENDED) {
			konafb_error("Not in LINK_SUSPENDED state (%d)\n",
							fb->blank_state);
			ret = -EINVAL;
			goto exit_unlock;
		}
		link_control(fb, RESUME_LINK);
		complete(&g_kona_fb->prev_buf_done_sem);
		kona_clock_start(fb);
		panel_write(fb->display_info->special_mode_off_seq);
		if (fb->display_info->cabc_enabled)
			panel_write(fb->display_info->cabc_on_seq);
		kona_clock_stop(fb);
		schedule_work(&fb->vsync_smart);
		if (fb->fb_data->esdcheck)
			queue_delayed_work(fb->esd_check_wq,
				&fb->esd_check_work,
				msecs_to_jiffies(
					fb->fb_data->esdcheck_period_ms));

		fb->display_info->special_mode_on = false;
		fb->blank_state = KONA_FB_UNBLANK;
		konafb_debug("Special mode OFF\n");
	}
exit_unlock:
	mutex_unlock(&fb->update_sem);
exit:
	return ret;
}

static struct device_attribute panel_attributes[] = {
	__ATTR(panel_name, S_IRUGO, kona_fb_panel_name_show, NULL),
	__ATTR(panel_id, S_IRUGO, kona_fb_panel_id_show, NULL),
	__ATTR(panel_mode, S_IRUGO|S_IWUSR|S_IWGRP,
					kona_fb_panel_mode_show,
					kona_fb_panel_mode_store),
};

static int register_attributes(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(panel_attributes); i++)
		if (device_create_file(dev, panel_attributes + i))
			goto error;
	return 0;
error:
	dev_err(dev, "%s: Unable to create interface\n", __func__);
	for (--i; i >= 0; i--)
		device_remove_file(dev, panel_attributes + i);
	return -ENODEV;
}

static void remove_attributes(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(panel_attributes); i++)
		device_remove_file(dev, panel_attributes + i);
}

#ifdef CONFIG_DEBUG_FS
/*
 * Usage:
 * echo <reg> <nbr_bytes> > panel_read
 * reg in hex
 * nbr_bytes in decimal
 * example: echo 0xDA 1 > panel_read
 */
static ssize_t dbgfs_reg_read(struct file *file, const char __user *ubuf,
						size_t count, loff_t *ppos)
{
	int ret = 0;
	struct seq_file *s = file->private_data;
	struct kona_fb *fb = s->private;
	int len;
	UInt8 *buff;
	uint8_t reg;
	char *kbuf = NULL;
	char *p;
	int i;

	kbuf = kzalloc(sizeof(char) * count, GFP_KERNEL);
	if (!kbuf) {
		pr_err("%s: Failed to allocate buffer\n", __func__);
		ret = -ENOMEM;
		goto exit;
	}

	if (copy_from_user(kbuf, ubuf, count)) {
		pr_err("%s: Failed to copy from user\n", __func__);
		ret = -EFAULT;
		goto fail_exit;
	}

	p = kbuf;
	p = p + 2;
	if (sscanf(p, "%2hhx", &reg) != 1) {
		pr_err("%s: Parameter error\n", __func__);
		ret = -EINVAL;
		goto fail_exit;
	}
	p = p + 2;
	if (sscanf(p, "%d", &len) != 1) {
		pr_err("%s: Parameter error\n", __func__);
		ret = -EINVAL;
		goto fail_exit;
	}

	if (len == 0) {
		pr_err("%s: Parameter error (len = 0)\n", __func__);
		ret = -EINVAL;
		goto fail_exit;
	}

	buff = kzalloc(sizeof(char) * len, GFP_KERNEL);
	if (!buff) {
		pr_err("%s: Failed to allocate buffer\n", __func__);
		ret = -ENOMEM;
		goto fail_exit;
	}

	if (fb->blank_state == KONA_FB_LINK_SUSPENDED) {
		konafb_debug("Resume link\n");
		link_control(fb, RESUME_LINK);
	}

	if (!fb->display_info->vmode)
		kona_clock_start(fb);

	panel_read(reg, buff, len);

	if (!fb->display_info->vmode)
		kona_clock_stop(fb);

	if (fb->blank_state == KONA_FB_LINK_SUSPENDED) {
		konafb_debug("Suspend link again\n");
		link_control(fb, SUSPEND_LINK);
	}

	pr_info("%s: reg 0x%.2x, len %d\n", __func__, reg, len);
	for (i = 0; i < len; i++)
		pr_info("%s: buff[%i] 0x%.2x\n", __func__, i, buff[i]);

	kfree(buff);
fail_exit:
	kfree(kbuf);
exit:
	return count;
}

static int dbgfs_read_open(struct inode *inode, struct file *file)
{
	return single_open(file, NULL, inode->i_private);
}

static const struct file_operations dbgfs_read_fops = {
	.owner			= THIS_MODULE,
	.open			= dbgfs_read_open,
	.write			= dbgfs_reg_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

/*
 * Usage:
 * echo <type> <reg> <data, data, ...> > panel_write
 * type: dcs gen
 * reg in hex
 * data in hex
 * example: echo dcs 0x29 0 > panel_write
 */
static ssize_t dbgfs_reg_write(struct file *file, const char __user *ubuf,
						size_t count, loff_t *ppos)
{
	int ret = 0;
	u8 reg;
	char *kbuf = NULL;
	char *p;
	u8 rec[DBGFS_MAX_BUFF_SIZE];
	int rec_len = 0;
	u8 data;
	struct seq_file *s = file->private_data;
	struct kona_fb *fb = s->private;

	kbuf = kzalloc(sizeof(char) * count, GFP_KERNEL);
	if (!kbuf) {
		pr_err("%s: Failed to allocate buffer\n", __func__);
		ret = -ENOMEM;
		goto exit;
	}

	if (copy_from_user(kbuf, ubuf, count)) {
		pr_err("%s: Failed to copy from user\n", __func__);
		ret = -EFAULT;
		goto fail_exit;
	}

	p = kbuf;

	/* Get type of command */
	if (!strncmp(kbuf, "dcs", 3)) {
		rec[0] = DISPCTRL_WR_CMND;
		rec_len = 1;
	} else if (!strncmp(kbuf, "gen", 3)) {
		rec[0] = DISPCTRL_TAG_GEN_WR;
		rec_len = 2; /* save one slot for length */
	} else {
		pr_err("%s: Parameter error\n", __func__);
		ret = -EFAULT;
		goto fail_exit;
	}
	p = p + 3;

	/* Get  to next parameter */
	while (isspace(*p) || *p == '0' || (*p == 'x')) {
		p++;
	}

	if (iscntrl(*p)) { /* end of string? */
		ret = -EINVAL;
		goto fail_exit;
	}

	/* Get register */
	if (sscanf(p, "%4hhx", &reg) != 1) {
		pr_err("%s: Parameter error\n", __func__);
		ret = -EINVAL;
		goto fail_exit;
	}
	rec[rec_len] = reg;
	rec_len++;

	while (!isspace(*p) && !(*p == ',') && !iscntrl(*p))
		p++;

	/* Get data */
	while (!iscntrl(*p)) {
		if (isspace(*p) || (*p == ',')) {
			p++;
		} else if (isxdigit(*p)) {
			if (sscanf(p, "%4hhx", &data) == 1) {
				rec[rec_len] = data;
				rec_len++;
				if (rec_len >= DBGFS_MAX_BUFF_SIZE) {
					pr_info("%s: too many parameters\n",
								__func__);
					ret = -EINVAL;
					goto fail_exit;
				}
			}
			while (!isspace(*p) && !(*p == ',') && !iscntrl(*p))
				p++;
		} else {
			pr_info("%s: parameter parsing error\n", __func__);
			ret = -EINVAL;
			goto fail_exit;
		}
	}

	rec[rec_len] = 0; /* record must end with null */
	if (rec[0] == DISPCTRL_WR_CMND) {
		/* DCS has length in idx 0. command is not included */
		rec[0] = rec_len - 1;
	} else {
		if (rec_len <= 3) {
			pr_info("%s: generic command must have one parameter\n",
								__func__);
			ret = -EINVAL;
			goto fail_exit;
		}
		/* GEN has command in idx 0 and length in idx 1 */
		rec[1] = rec_len - 2;
	}

	if (fb->blank_state == KONA_FB_LINK_SUSPENDED) {
		konafb_debug("Resume link\n");
		link_control(fb, RESUME_LINK);
	}

	panel_write(rec);

	if (fb->blank_state == KONA_FB_LINK_SUSPENDED) {
		konafb_debug("Suspend link again\n");
		link_control(fb, SUSPEND_LINK);
	}

fail_exit:
	kfree(kbuf);
exit:
	return count;
}

static int dbgfs_write_open(struct inode *inode, struct file *file)
{
	return single_open(file, NULL, inode->i_private);
}

static const struct file_operations dbgfs_write_fops = {
	.owner			= THIS_MODULE,
	.open			= dbgfs_write_open,
	.write			= dbgfs_reg_write,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static ssize_t dbgfs_cabc_write(struct file *file, const char __user *ubuf,
						size_t count, loff_t *ppos)
{
	int ret = 0;
	char *kbuf = NULL;
	struct seq_file *s = file->private_data;
	struct kona_fb *fb = s->private;

	kbuf = kzalloc(sizeof(char) * count, GFP_KERNEL);
	if (!kbuf) {
		pr_err("%s: Failed to allocate buffer\n", __func__);
		ret = -ENOMEM;
		goto exit;
	}

	if (copy_from_user(kbuf, ubuf, count)) {
		pr_err("%s: Failed to copy from user\n", __func__);
		ret = -EFAULT;
		goto fail_exit;
	}

	if (!strncmp(kbuf, "0", 1)) {
		fb->display_info->cabc_enabled = false;
		fb->display_info->clear_panel_ram = false;
		if (fb->blank_state != KONA_FB_BLANK) {
			(void)kona_fb_blank(FB_BLANK_NORMAL, &fb->fb);
			(void)kona_fb_blank(FB_BLANK_UNBLANK, &fb->fb);
		}
		pr_info("%s: CABC turned off\n", __func__);
	} else {
		fb->display_info->cabc_enabled = true;
		fb->display_info->clear_panel_ram = true;
		if (fb->blank_state != KONA_FB_BLANK) {
			(void)kona_fb_blank(FB_BLANK_NORMAL, &fb->fb);
			(void)kona_fb_blank(FB_BLANK_UNBLANK, &fb->fb);
		}
		pr_info("%s: CABC turned on\n", __func__);
	}
fail_exit:
	kfree(kbuf);
exit:
	return count;
}

static int dbgfs_cabc_open(struct inode *inode, struct file *file)
{
	return single_open(file, NULL, inode->i_private);
}

static const struct file_operations dbgfs_cabc_fops = {
	.owner			= THIS_MODULE,
	.open			= dbgfs_cabc_open,
	.write			= dbgfs_cabc_write,
	.llseek			= seq_lseek,
	.release		= single_release,
};

void __init kona_fb_create_debugfs(struct platform_device *pdev)
{
	struct kona_fb *fb;
	struct device *dev;

	if (!pdev) {
		pr_err("%s: no device\n", __func__);
		return;
	}

	dev = &pdev->dev;

	fb = platform_get_drvdata(pdev);
	if (!fb) {
		dev_err(dev, "%s: failed to get drvdata\n", __func__);
		return;
	}
	if (!&dev->kobj) {
		dev_err(dev, "%s: no &dev->kobj\n", __func__);
		return;
	}

	fb->dbgfs_dir = debugfs_create_dir(kobject_name(&dev->kobj), 0);
	if (!fb->dbgfs_dir) {
		dev_err(dev, "%s: dbgfs create dir failed\n", __func__);
	} else {
		if (!debugfs_create_file("panel_read", S_IWUSR, fb->dbgfs_dir,
							fb, &dbgfs_read_fops)) {
			dev_err(dev, "%s: failed to create dbgfs read file\n",
								__func__);
			return;
		}
		if (!debugfs_create_file("panel_write", S_IWUSR, fb->dbgfs_dir,
						fb, &dbgfs_write_fops)) {
			dev_err(dev, "%s: failed to create dbgfs write file\n",
								__func__);
			return;
		}
		if (!debugfs_create_file("cabc", S_IWUSR, fb->dbgfs_dir,
						fb, &dbgfs_cabc_fops)) {
			dev_err(dev, "%s: failed to create dbgfs cabc file\n",
								__func__);
			return;
		}
	}
}

void __exit kona_fb_remove_debugfs(struct platform_device *pdev)
{
	struct kona_fb *fb;

	if (!pdev || !&pdev->dev) {
		pr_err("%s: no device\n", __func__);
		return;
	}

	fb = platform_get_drvdata(pdev);
	if (!fb) {
		dev_err(&pdev->dev, "%s: failed to get drvdata\n", __func__);
		return;
	}
	debugfs_remove_recursive(fb->dbgfs_dir);
}
#endif /* CONFIG_DEBUG_FS */

static int enable_display(struct kona_fb *fb)
{
	int ret = 0;

	ret = fb->display_ops->init(fb->display_info, &fb->display_hdl);
	if (ret != 0) {
		konafb_error("Failed to init this display device!\n");
		goto fail_to_init;
	}

	kona_clock_start(fb);
	ret = fb->display_ops->open(fb->display_hdl);
	if (ret != 0) {
		konafb_error("Failed to open this display device!\n");
		goto fail_to_open;
	}
	if (fb->fb_data->pm_sleep)
		ret = fb->display_ops->power_control(fb->display_hdl,
				CTRL_SLEEP_OUT);
	else
		ret = fb->display_ops->power_control(fb->display_hdl,
				CTRL_PWR_ON);
	if (ret != 0) {
		konafb_error("Failed to power on this display device!\n");
		goto fail_to_power_control;
	}
	INIT_WORK(&fb->vsync_smart, vsync_work_smart);
	if (!fb->display_info->vmode) {
		schedule_work(&fb->vsync_smart);
		kona_clock_stop(fb);
	}

	konafb_debug("kona display is enabled successfully\n");
	return 0;

fail_to_power_control:
	fb->display_ops->close(fb->display_hdl);
fail_to_open:
	kona_clock_stop(fb);
	fb->display_ops->exit(fb->display_hdl);
fail_to_init:
	return ret;

}

static int disable_display(struct kona_fb *fb)
{
	int ret = 0;

	if (!fb->display_info->vmode)
		kona_clock_start(fb);
	cancel_work_sync(&fb->vsync_smart);

	if (fb->fb_data->pm_sleep)
		fb->display_ops->power_control(fb->display_hdl,
			CTRL_SLEEP_IN);
	else
		fb->display_ops->power_control(fb->display_hdl,
			CTRL_PWR_OFF);
	fb->display_ops->close(fb->display_hdl);
	fb->display_ops->exit(fb->display_hdl);
	kona_clock_stop(fb);
	konafb_debug("kona display is disabled successfully\n");
	return ret;
}

static int kona_fb_ioctl(struct fb_info *info, unsigned int cmd,
			 unsigned long arg)
{
	void *ptr = NULL;
	int ret = 0;
	struct kona_fb *fb = container_of(info, struct kona_fb, fb);

	konafb_debug("kona ioctl called! Cmd %x, Arg %lx\n", cmd, arg);
	switch (cmd) {

	case FBIO_WAITFORVSYNC:
		if (wait_for_completion_killable(&vsync_event) < 0) {
			konafb_info("Failed to get a vsync event\n");
			ret = -ETIMEDOUT;
		}
		break;

	case KONA_IOCTL_SET_BUFFER_AND_UPDATE:
		if (mutex_lock_killable(&fb->update_sem))
			return -EINTR;
		ptr = (void *)arg;

		if (ptr == NULL) {
			mutex_unlock(&fb->update_sem);
			return -EFAULT;
		}

		if (!fb->display_info->vmode) {
			if (wait_for_completion_timeout(
			&fb->prev_buf_done_sem,	msecs_to_jiffies(10000)) <= 0)
				pr_err("%s:%d timed out waiting for completion",
					__func__, __LINE__);
			kona_clock_start(fb);
			ret = fb->display_ops->update(
					fb->display_hdl, ptr, NULL, NULL);
			kona_clock_stop(fb);
			complete(&g_kona_fb->prev_buf_done_sem);
		} else {
			ret = fb->display_ops->update(
					fb->display_hdl, ptr, NULL, NULL);
		}

		mutex_unlock(&fb->update_sem);
		break;

	case KONA_IOCTL_GET_FB_IOVA:
		ptr = (void *)arg;
		if (ptr == NULL) {
			pr_err("arg=NULL\n");
			return -EFAULT;
		}
		ret = copy_to_user(ptr, &fb->buff0, sizeof(u32));
		if (ret < 0)
			pr_err("copy2user failed ret=%d\n", ret);
		break;

	default:
		konafb_error("Wrong ioctl cmd\n");
		ret = -ENOTTY;
		break;
	}

	return ret;
}

static int clear_panel_ram(struct kona_fb *fb)
{
	int ret = 0;
	DISPDRV_WIN_t p_win;
	void *black_buf;
	size_t fsize, black_ram_size;
	dma_addr_t phys_base;
	struct platform_device *pdev = fb->pdev;
	uint16_t black_ram_lines;

	pr_info("%s: fill RAM with black\n", __func__);
	/* Workaround: One page extra allocated and mapped via m4u to avoid
	* v3d write faulting in m4u doing extra access */
	black_ram_lines = fb->display_info->clear_ram_row_end -
				fb->display_info->clear_ram_row_start + 1;
	black_ram_size = fb->display_info->width * black_ram_lines *
							fb->display_info->Bpp;
	fsize = PAGE_ALIGN(4096 + black_ram_size);

	black_buf = dma_alloc_writecombine(&pdev->dev, fsize, &phys_base,
								GFP_KERNEL);
	if (black_buf == NULL) {
		ret = -ENOMEM;
		konafb_error("Unable to allocate black framebuffer\n");
	} else {
		memset(black_buf, 0, fsize);
		p_win.l = 0;
		p_win.r = fb->display_info->width - 1;
		p_win.t = fb->display_info->clear_ram_row_start - 1;
		p_win.b = fb->display_info->clear_ram_row_end - 1;
		p_win.h = fb->display_info->clear_ram_row_end;
		p_win.w = fb->display_info->width;
		ret = fb->display_ops->update(fb->display_hdl, black_buf,
								&p_win, NULL);
		dma_free_writecombine(&pdev->dev, fsize, black_buf, phys_base);
	}
	return ret;
}

static int kona_fb_blank(int blank_mode, struct fb_info *info)
{
	struct kona_fb *fb = container_of(info, struct kona_fb, fb);

	switch (blank_mode) {
	case FB_BLANK_POWERDOWN:
	case FB_BLANK_HSYNC_SUSPEND:
	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_NORMAL:
		mutex_lock(&fb->update_sem);
		if (fb->blank_state == KONA_FB_BLANK) {
			konafb_error("Display already in blank state\n");
			mutex_unlock(&fb->update_sem);
			break;
		}

		if (fb->blank_state == KONA_FB_LINK_SUSPENDED) {
			konafb_debug("Enter black mode\n");
			link_control(fb, RESUME_LINK);
		}

		if (fb->fb_data->esdcheck) {
			/* cancel the esd check routine */
			cancel_delayed_work(&fb->esd_check_work);
		}

		fb->g_stop_drawing = 1;
		/* In case of video mode, DSI commands can be sent out-of-sync
		 * of buffers */
		if (!fb->display_info->vmode) {
			if (wait_for_completion_timeout(
			&fb->prev_buf_done_sem,	msecs_to_jiffies(10000)) <= 0)
				pr_err("%s:%d timed out waiting for completion",
					__func__, __LINE__);
			kona_clock_start(fb);
			if (fb->display_ops->power_control(fb->display_hdl,
					       CTRL_SCREEN_OFF))
				konafb_error(
				"Failed to blank this display device!\n");
			kona_clock_stop(fb);
			complete(&g_kona_fb->prev_buf_done_sem);
		} else {
			if (fb->display_ops->power_control(fb->display_hdl,
					       CTRL_SCREEN_OFF))
				konafb_error(
				"Failed to blank this display device!\n");
		}

		/* screen goes to sleep mode */
		disable_display(fb);
		mutex_unlock(&fb->update_sem);
		/* Ok for MM going to shutdown state */
		pi_mgr_qos_request_update(&g_mm_qos_node,
					  PI_MGR_QOS_DEFAULT_VALUE);

		fb->blank_state = KONA_FB_BLANK;
		break;

	case FB_BLANK_UNBLANK:
		mutex_lock(&fb->update_sem);
		if (fb->blank_state == KONA_FB_UNBLANK) {
			konafb_error("Display already in unblank state\n");
			mutex_unlock(&fb->update_sem);
			break;
		}
		if (fb->blank_state == KONA_FB_LINK_SUSPENDED) {
			konafb_error("Display in link suspended state\n");
			mutex_unlock(&fb->update_sem);
			break;
		}
		/* Ok for MM going to retention but not shutdown state */
		pi_mgr_qos_request_update(&g_mm_qos_node, 10);
		/* screen comes out of sleep */
		if (enable_display(fb))
			konafb_error("Failed to enable this display device\n");

		if (!fb->display_info->vmode) {
			if (wait_for_completion_timeout(
			&fb->prev_buf_done_sem,	msecs_to_jiffies(10000)) <= 0)
				pr_err("%s:%d timed out waiting for completion",
					__func__, __LINE__);

			kona_clock_start(fb);
		}
		if (fb->display_info->clear_panel_ram)
			(void)clear_panel_ram(fb);
		fb->display_ops->update(fb->display_hdl,
				fb->fb.var.yoffset ? fb->buff1 : fb->buff0,
				NULL,
				(DISPDRV_CB_T)kona_display_done_cb);

		if (fb->display_info->vmode) {
			if (wait_for_completion_timeout(
			&fb->prev_buf_done_sem,	msecs_to_jiffies(10000)) <= 0)
				pr_err("%s:%d timed out waiting for completion",
					__func__, __LINE__);
		}


		fb->g_stop_drawing = 0;

		if (fb->display_info->special_mode_on == true) {
			konafb_debug("Exit black mode. Go to special mode\n");
			kona_clock_start(fb);
			panel_write(fb->display_info->special_mode_on_seq);
			kona_clock_stop(fb);
		}

		if (!fb->display_info->vmode) {
			kona_clock_start(fb);

			if (fb->display_ops->
			    power_control(fb->display_hdl, CTRL_SCREEN_ON))
				konafb_error
				("Failed to unblank this display device!\n");

			kona_clock_stop(fb);
		} else {
			if (fb->display_ops->
			    power_control(fb->display_hdl, CTRL_SCREEN_ON))
				konafb_error
				("Failed to unblank this display device!\n");
		}
#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
		if (atomic_read(&g_kona_fb->force_update))
			kona_display_crash_image(CP_CRASH_DUMP_START);
#endif
		if (fb->fb_data->esdcheck) {
			/* schedule the esd check routine */
			queue_delayed_work(fb->esd_check_wq,
					&fb->esd_check_work,
			msecs_to_jiffies(fb->fb_data->esdcheck_period_ms));
		}

		if (fb->display_info->special_mode_on == true) {
			konafb_debug("Exit black mode. Go to link suspend\n");
			link_control(fb, SUSPEND_LINK);
			fb->blank_state = KONA_FB_LINK_SUSPENDED;
		} else {
			fb->blank_state = KONA_FB_UNBLANK;
		}
		mutex_unlock(&fb->update_sem);
		break;

	default:
		return 1;
	}

	return 0;
}

static irqreturn_t kona_fb_esd_irq(int irq, void *dev_id)
{
	struct kona_fb *fb = (struct kona_fb *)dev_id;
	complete(&fb->tectl_gpio_done_sem);
	return IRQ_HANDLED;
}



static void kona_fb_esd_check(struct work_struct *data)
{
	struct kona_fb *fb = container_of((struct delayed_work *)data,
			struct kona_fb, esd_check_work);
	int esd_result = -EIO;
	uint32_t tectl_gpio = fb->fb_data->tectl_gpio;

	if (g_display_enabled) {
		/* Wait fb initial completed */
		queue_delayed_work(fb->esd_check_wq, &fb->esd_check_work,
			msecs_to_jiffies(fb->fb_data->esdcheck_period_ms));
		return;
	}

	if (fb->g_stop_drawing)
		return;

	if (tectl_gpio > 0) {
		unsigned long jiff_in = 0;

		/* mark as incomplete before waiting the esd signal */
		INIT_COMPLETION(fb->tectl_gpio_done_sem);
		enable_irq(gpio_to_irq(tectl_gpio));
		jiff_in = wait_for_completion_timeout(&fb->tectl_gpio_done_sem,
				msecs_to_jiffies(200));
		if (!jiff_in) {
			konafb_error("Wait esd gpio irq timeout\n");
			esd_result = -EIO;
		} else
			esd_result = 0;
		disable_irq(gpio_to_irq(tectl_gpio));
	} else {
		mutex_lock(&fb->update_sem);
		if (fb->display_info->esd_check_fn)
			esd_result = fb->display_info->esd_check_fn();
		if (esd_result < 0) /* stop drawing till error is recovered */
			fb->g_stop_drawing = 1;
		mutex_unlock(&fb->update_sem);
	}

	konafb_debug("esd_result %d\n", esd_result);

	if (esd_result < 0) {
		konafb_error("result %d failure_cnt %d\n", esd_result,
				fb->esd_failure_cnt);

		fb->esd_failure_cnt++;
		if (fb->esd_failure_cnt > fb->fb_data->esdcheck_retry) {
			konafb_error("too many fail %d, disable esd check\n",
					fb->esd_failure_cnt);
			fb->fb_data->esdcheck = FALSE;
			cancel_delayed_work(&fb->esd_check_work);
			destroy_workqueue(fb->esd_check_wq);
		}
	} else {
		fb->esd_failure_cnt = 0;
		queue_delayed_work(fb->esd_check_wq, &fb->esd_check_work,
			msecs_to_jiffies(fb->fb_data->esdcheck_period_ms));
	}

	return;
}

void free_platform_data(struct device *dev)
{
	if (dev->of_node)
		kfree(dev->platform_data);
}

static int __init lcd_panel_setup(char *panel)
{
	if (panel && strlen(panel)) {
		pr_err("bootloader has initialised %s\n", panel);
		strcpy(g_disp_str, panel);
		g_display_enabled = 0;
	}
	return 1;
}
__setup("lcd_panel=", lcd_panel_setup);


static int __init lcd_boot_mode(char *mode)
{
#ifndef CONFIG_FB_NEED_PAGE_ALIGNMENT
	if (mode && strlen(mode)) {
		if (!strcmp(mode, "recovery") || !strcmp(mode, "charger"))
			need_page_alignment = 0;
	}
#endif
	return 1;
}
__setup("androidboot.mode=", lcd_boot_mode);

#ifdef CONFIG_OF
static struct kona_fb_platform_data * __init get_of_data(struct device_node *np)
{
	u32 val;
	const char *str;
	struct kona_fb_platform_data *fb_data;
#ifdef CONFIG_IOMMU_API
	struct device_node *tmp_node;
#endif

	fb_data = kzalloc(sizeof(struct kona_fb_platform_data),
		GFP_KERNEL);
	if (!fb_data) {
		pr_err("couldn't allocate memory for pdata");
		goto alloc_failed;
	}

	if (of_property_read_string(np,	"module-name", &str))
		goto of_fail;
	if (unlikely(strlen(str) > DISPDRV_NAME_SZ))
		goto of_fail;
	if (g_display_enabled && strcmp(str, g_disp_str)) {
		pr_err("%s != %s enabled by bootloader\n", str, g_disp_str);
		goto of_fail;
	}
	strcpy(fb_data->name, str);

	if (of_property_read_string(np, "reg-name", &str))
		goto of_fail;
	if (unlikely(strlen(str) > DISPDRV_NAME_SZ))
		goto of_fail;
	strcpy(fb_data->reg_name, str);
	fb_data->rst.gpio = of_get_named_gpio(np, "rst-gpio", 0);
	if (!gpio_is_valid(fb_data->rst.gpio))
		goto of_fail;
	if (of_property_read_u32(np, "rst-setup", &val))
		goto of_fail;
	fb_data->rst.setup = val;
	if (of_property_read_u32(np, "rst-pulse", &val))
		goto of_fail;
	fb_data->rst.pulse = val;
	if (of_property_read_u32(np, "rst-hold", &val))
		goto of_fail;
	fb_data->rst.hold = val;

	if (of_property_read_bool(np, "rst-active-high"))
		fb_data->rst.active = true;
	else
		fb_data->rst.active = false;

	if (of_property_read_u32(np, "rotation", &val))
		goto of_fail;
	fb_data->rotation = val;

	fb_data->detect.gpio = of_get_named_gpio(np, "detect-gpio", 0);
	if (gpio_is_valid(fb_data->detect.gpio)) {
		if (of_property_read_u32(np,
			"detect-gpio-val", &val))
			goto of_fail;
		fb_data->detect.gpio_val = val;

		fb_data->detect.active =
			gpio_get_value(fb_data->detect.gpio);
		if (fb_data->detect.active !=
				fb_data->detect.gpio_val) {
			konafb_error(
			"gpio %d value failed for panel %s\n",
			fb_data->detect.gpio, fb_data->name);
			goto of_fail;
		}
	}

	if (of_property_read_bool(np, "vmode"))
		fb_data->vmode = true;
	else
		fb_data->vmode = false;
	if (of_property_read_bool(np, "vburst"))
		fb_data->vburst = true;
	else
		fb_data->vburst = false;
	if (of_property_read_bool(np, "cmnd-LP"))
		fb_data->cmnd_LP = true;
	else
		fb_data->cmnd_LP = false;
	if (of_property_read_bool(np, "te-ctrl"))
		fb_data->te_ctrl = true;
	else
		fb_data->te_ctrl = false;
	if (of_property_read_bool(np, "pm-sleep"))
		fb_data->pm_sleep = true;
	else
		fb_data->pm_sleep = false;

	if (of_property_read_u32(np, "col-mod-i", &val))
		goto of_fail;
	fb_data->col_mod_i = (uint8_t)val;
	if (of_property_read_u32(np, "col-mod-o", &val))
		goto of_fail;
	fb_data->col_mod_o = (uint8_t)val;
	if (of_property_read_u32(np, "width", &val))
		goto of_fail;
	fb_data->width = (uint16_t)val;
	if (of_property_read_u32(np, "height", &val))
		goto of_fail;
	fb_data->height = (uint16_t)val;
	if (of_property_read_u32(np, "fps", &val))
		goto of_fail;
	fb_data->fps = (uint8_t)val;
	if (of_property_read_u32(np, "lanes", &val))
		goto of_fail;
	fb_data->lanes = (uint8_t)val;
	if (of_property_read_u32(np, "hs-bitrate", &val))
		goto of_fail;
	fb_data->hs_bps = val;
	if (of_property_read_u32(np, "lp-bitrate", &val))
		goto of_fail;
	fb_data->lp_bps = val;

	/* Desense offset value to be absolute value w.r.t hs-bitrate */
	if (of_property_read_u32(np, "desense-offset", &val)) {
		konafb_info("desense offset not populated\n");
		fb_data->desense_offset = 0;
	} else {
		konafb_info("desense offset requested %d\n", val);
		fb_data->desense_offset = (int) val;
	}

	/*Get the ESD config */
	if (of_property_read_bool(np, "esdcheck")) {
		fb_data->esdcheck = TRUE;
		if (of_property_read_u32(np, "esdcheck-period-ms", &val))
			goto of_fail;
		fb_data->esdcheck_period_ms = val;
		if (of_property_read_u32(np, "esdcheck-retry", &val))
			goto of_fail;
		fb_data->esdcheck_retry = (uint8_t)val;
		if (of_property_read_u32(np, "tectl-gpio", &val))
			fb_data->tectl_gpio = 0;
		else {
			/* Only allow tectl as gpio if TE is not used */
			if (fb_data->vmode || !fb_data->te_ctrl)
				fb_data->tectl_gpio = val;
			else
				konafb_info("Can't enable tectl_gpio");
		}
	} else
		fb_data->esdcheck = FALSE;

#ifdef CONFIG_IOMMU_API
	/* Get the iommu device and link fb dev to iommu dev */
	tmp_node = of_parse_phandle(np, "iommu", 0);
	if (tmp_node  == NULL) {
		pr_err("%s get node(iommu) failed\n", __func__);
		goto of_fail;
	}
	fb_data->pdev_iommu = of_find_device_by_node(tmp_node);
	if (fb_data->pdev_iommu == NULL) {
		pr_err("%s get iommu device failed\n", __func__);
		goto of_fail;
	}
#endif /* CONFIG_IOMMU_API */

#ifdef CONFIG_BCM_IOVMM
	/* Get the iommu mapping and attach fb dev to mapping */
	tmp_node = of_parse_phandle(np,	"iovmm", 0);
	if (tmp_node  == NULL) {
		pr_err("%s get node(iovmm) failed\n", __func__);
		goto of_fail;
	}
	fb_data->pdev_iovmm = of_find_device_by_node(tmp_node);
	if (fb_data->pdev_iovmm == NULL) {
		pr_err("%s get iovmm device failed\n", __func__);
		goto of_fail;
	}
#endif /* CONFIG_BCM_IOVMM */


	return fb_data;

of_fail:
	konafb_error("get_of_data failed\n");
	kfree(fb_data);
alloc_failed:
	return NULL;
}
#endif /* CONFIG_OF */

static char *get_seq(DISPCTRL_REC_T *rec)
{
	char *buff, *dst;
	int list_len, cmd_len;
	DISPCTRL_REC_T *cur;
	int i;

	buff = NULL;
	/* Get the length of sequence in bytes = cmd+data+headersize+null */
	cur = rec;
	list_len = 0;
	for (i = 0; DISPCTRL_LIST_END != cur[i].type; i++) {
		if (DISPCTRL_WR_DATA == cur[i].type)
			list_len++;
		else if (DISPCTRL_GEN_WR_CMND == cur[i].type)
			list_len += 3; /* One more tag for Gen cmd */
		else
			list_len += 2; /* Indicates new packet */
	}

	list_len++; /* NULL termination */

	/* Allocate buff = length */
	buff = kmalloc(list_len, GFP_KERNEL);
	if (!buff)
		goto seq_done;


	/* Parse the DISPCTRL_REC_T[], extract data and fill buff */
	cur = rec;
	dst = buff;
	i = 0;
	for (i = 0; DISPCTRL_LIST_END != cur[i].type; i++) {
		switch (cur[i].type) {
		case DISPCTRL_GEN_WR_CMND:
			*dst++ = DISPCTRL_TAG_GEN_WR;
			/* fall through */
		case DISPCTRL_WR_CMND:
			cmd_len = 1;
			dst++;
			*dst++ = cur[i].val;
			while (DISPCTRL_WR_DATA == cur[i + 1].type) {
				i++;
				*dst++ = cur[i].val;
				cmd_len++;
			}
			if (cmd_len > DISPCTRL_MAX_DATA_LEN) {
				pr_err("cmd_len %d reach max\n", cmd_len);
				kfree(buff);
				buff = NULL;
				goto seq_done;
			}
			*(dst - cmd_len - 1) = cmd_len;
			break;
		case DISPCTRL_SLEEP_MS:
			/* Maximum packet size is limited to 254 */
			*dst++ = DISPCTRL_TAG_SLEEP;
			*dst++ = cur[i].val;
			break;
		default:
			pr_err("Invalid control list type %d\n", cur[i].type);
		}
	}

	/* Put a NULL at the end */
	*dst = 0;
	if (dst != (buff + list_len - 1))
		pr_err("dst ptr mismatch\n");
#if 0
	pr_err("dst %p buff %p\n", dst, buff);
	dst = buff;
	while (*dst)
		pr_err("%d ", *dst++);
	pr_err("list end size %d\n", list_len);
#endif
seq_done:
	return buff;
}

static int __init populate_dispdrv_cfg(struct kona_fb *fb,
	struct kona_fb_platform_data *pd)
{
	struct lcd_config *cfg;
	DISPDRV_INFO_T *info;
	int ret = -1;

	cfg = get_dispdrv_cfg(pd->name);
	if (!cfg) {
		pr_err("Couldn't find a suitable dispdrv\n");
		ret = -ENXIO;
		goto err_cfg;
	}

	if (cfg->mode_supp != LCD_CMD_VID_BOTH) {
		if (pd->vmode && (cfg->mode_supp != LCD_VID_ONLY)) {
			pr_err("No vid mode support\n");
			ret = -EINVAL;
			goto err_cfg;
		}
		if (!pd->vmode && (cfg->mode_supp != LCD_CMD_ONLY)) {
			pr_err("No cmd mode support\n");
			ret = -EINVAL;
			goto err_cfg;
		}
	}

	/* Allocate memory for disp_info */
	/* Setting memory to zero is mandatory for this struct */
	info = kzalloc(sizeof(DISPDRV_INFO_T), GFP_KERNEL);
	if (!info) {
		pr_err("Failed to allocate memory fr disp_info\n");
		ret = -ENOMEM;
		goto err_mem_disp_info;
	}

	/* Fill up "info" using "cfg" and "pd" */
	info->name = pd->name;
	info->reg_name = pd->reg_name;
	info->rst = &pd->rst;
	info->vmode = pd->vmode;
	info->vburst = (pd->vburst == cfg->vburst) ? pd->vburst : false;
	info->vid_cmnds = cfg->vid_cmnds;
	info->cmnd_LP = pd->cmnd_LP;
	info->te_ctrl = pd->te_ctrl;
	info->width = pd->width;
	info->height = pd->height;
	info->lanes = (pd->lanes > cfg->max_lanes) ? cfg->max_lanes : pd->lanes;

	/* Hardcode for now */
	info->in_fmt = pd->col_mod_i;
	info->out_fmt = pd->col_mod_o;
	if (info->in_fmt == DISPDRV_FB_FORMAT_RGB666U ||
			info->in_fmt == DISPDRV_FB_FORMAT_RGB666P ||
			info->in_fmt == DISPDRV_FB_FORMAT_RGB565)
		info->Bpp = 2;
	else
		info->Bpp = 4;

	info->phys_width = cfg->phys_width;
	info->phys_height = cfg->phys_height;
	info->fps = pd->fps;

	info->slp_in_seq = get_seq(cfg->slp_in_seq);
	if (!info->slp_in_seq)
		goto err_slp_in_seq;
	info->slp_out_seq = get_seq(cfg->slp_out_seq);
	if (!info->slp_out_seq)
		goto err_slp_out_seq;
	info->scrn_on_seq = get_seq(cfg->scrn_on_seq);
	if (!info->scrn_on_seq)
		goto err_scrn_on_seq;
	info->scrn_off_seq = get_seq(cfg->scrn_off_seq);
	if (!info->scrn_off_seq)
		goto err_scrn_off_seq;
	if (cfg->verify_id) {
		info->id_seq = get_seq(cfg->id_seq);
		if (!info->id_seq)
			goto err_id_seq;
	}
	memcpy(info->phy_timing, cfg->phy_timing, sizeof(info->phy_timing));
	if (info->vmode) {
		info->init_seq = get_seq(cfg->init_vid_seq);
		info->hs = cfg->hs;
		info->hbp = cfg->hbp;
		info->hfp = cfg->hfp;
		info->hbllp = cfg->hbllp;
		info->vs = cfg->vs;
		info->vbp = cfg->vbp;
		info->vfp = cfg->vfp;
	} else {
		info->init_seq = get_seq(cfg->init_cmd_seq);
		info->updt_win_fn = cfg->updt_win_fn;
		info->updt_win_seq_len = cfg->updt_win_seq_len;
	}
	if (!info->init_seq)
		goto err_init_seq;

	if (!info->vmode) {
		if (!info->updt_win_seq_len) {
			pr_err("Abort: win_seq_len = 0 for command mode\n");
			goto err_win_seq;
		}
		info->win_seq = kmalloc(info->updt_win_seq_len, GFP_KERNEL);
		if (!info->win_seq)
			goto err_win_seq;
	}

	/* Panel special mode */
	if (cfg->special_mode_panel) {
		info->special_mode_on_seq =
					get_seq(cfg->special_mode_on_cmd_seq);
		if (!info->special_mode_on_seq)
			goto err_special_mode_seq;
		info->special_mode_off_seq =
					get_seq(cfg->special_mode_off_cmd_seq);
		if (!info->special_mode_off_seq)
			goto err_special_mode_seq;
		info->special_mode_on = cfg->special_mode_on;
		info->special_mode_panel = cfg->special_mode_panel;
		pr_info("%s(%d): Panel special mode: %d\n",
				__func__, __LINE__, info->special_mode_on);
	}

	info->cabc_enabled = cfg->cabc_enabled;
	if (info->cabc_enabled) {
		info->cabc_init_seq = get_seq(cfg->cabc_init_seq);
		if (!info->cabc_init_seq)
			goto err_cabc_seq;
		info->cabc_on_seq = get_seq(cfg->cabc_on_seq);
		if (!info->cabc_on_seq)
			goto err_cabc_seq;
		info->cabc_off_seq = get_seq(cfg->cabc_off_seq);
		if (!info->cabc_off_seq)
			goto err_cabc_seq;
	}

	info->clear_panel_ram = cfg->clear_panel_ram;
	info->clear_ram_row_start = cfg->clear_ram_row_start;
	info->clear_ram_row_end = cfg->clear_ram_row_end;
	info->no_te_in_sleep = cfg->no_te_in_sleep;

	if (info->clear_panel_ram)	/* Override kernel command line ... */
		g_display_enabled = 0;	/* ... and force re-init of display */


	/* burst mode changes to be taken care here or PV? */
	info->hs_bps = (pd->hs_bps > cfg->max_hs_bps) ?
				 cfg->max_hs_bps : pd->hs_bps;
	info->lp_bps = (pd->lp_bps > cfg->max_lp_bps) ?
				 cfg->max_lp_bps : pd->lp_bps;
	info->desense_offset = pd->desense_offset;

	info->vsync_cb = (info->vmode) ? konafb_vsync_cb : NULL;
	info->cont_clk = cfg->cont_clk;
	info->init_fn = cfg->init_fn;
	info->esd_check_fn = cfg->esd_check_fn;
	fb->display_info = info;
	return 0;

err_cabc_seq:
	kfree(info->cabc_init_seq);
	kfree(info->cabc_on_seq);
	kfree(info->cabc_off_seq);
err_special_mode_seq:
	kfree(info->special_mode_on_seq);
	kfree(info->special_mode_off_seq);
err_win_seq:
	kfree(info->init_seq);
err_init_seq:
	if (cfg->verify_id)
		kfree(info->id_seq);
err_id_seq:
	kfree(info->scrn_off_seq);
err_scrn_off_seq:
	kfree(info->scrn_on_seq);
err_scrn_on_seq:
	kfree(info->slp_out_seq);
err_slp_out_seq:
	kfree(info->slp_in_seq);
err_slp_in_seq:
	kfree(info);
err_mem_disp_info:
err_cfg:
	return ret;
}

void release_dispdrv_info(DISPDRV_INFO_T *info)
{
	BUG_ON(ZERO_OR_NULL_PTR(info));
	kfree(info->cabc_init_seq);
	kfree(info->cabc_on_seq);
	kfree(info->cabc_off_seq);
	kfree(info->special_mode_on_seq);
	kfree(info->special_mode_off_seq);
	kfree(info->init_seq);
	kfree(info->slp_in_seq);
	kfree(info->slp_out_seq);
	kfree(info->scrn_on_seq);
	kfree(info->scrn_off_seq);
	kfree(info->win_seq);
	kfree(info);
}

static int kona_fb_open(struct fb_info *info, int user)
{
	struct kona_fb *fb = container_of(info, struct kona_fb, fb);
	fb->open_count++;
	return 0;
}

static int kona_fb_release(struct fb_info *info, int user)
{
	struct kona_fb *fb = container_of(info, struct kona_fb, fb);
	fb->open_count--;

	if (fb->open_count < 0)
		return -EIO;
	/* If the open count goes 0, then restore the rotation parameter so that
	 * user space can again get to know the rotation parameter. */
	if (fb->open_count == 0)
		info->var.rotate = fb->rotate;

	return 0;
}

static struct fb_ops kona_fb_ops = {
	.fb_open = kona_fb_open,
	.fb_release = kona_fb_release,
	.owner = THIS_MODULE,
	.fb_check_var = kona_fb_check_var,
	.fb_set_par = kona_fb_set_par,
	.fb_setcolreg = kona_fb_setcolreg,
	.fb_pan_display = kona_fb_pan_display,
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
	.fb_ioctl = kona_fb_ioctl,
	.fb_sync = kona_fb_sync,
	.fb_blank = kona_fb_blank,
};


static const struct file_operations proc_fops = {
	.write = proc_write_fb_test,
};

static int __ref kona_fb_probe(struct platform_device *pdev)
{
	int ret = -ENXIO;
	struct kona_fb *fb;
	size_t framesize, framesize_alloc;
	uint32_t width, height;
	int ret_val = -1;
	struct kona_fb_platform_data *fb_data;
	dma_addr_t phys_fbbase, dma_addr, direct_dma_addr;
	uint64_t pixclock_64;
#ifdef CONFIG_LOGO
	int logo_rotate;
#endif
	uint32_t uboot_phys;
	void *uboot_kvirt = NULL;
	int need_map_switch = 0;

	konafb_info("start\n");
	if (g_kona_fb && (g_kona_fb->is_display_found == 1)) {
		konafb_info("A right display device is already found!\n");
		return -EINVAL;
	}

	fb = kzalloc(sizeof(struct kona_fb), GFP_KERNEL);
	if (fb == NULL) {
		konafb_error("Unable to allocate framebuffer structure\n");
		ret = -ENOMEM;
		goto err_fb_alloc_failed;
	}
	fb->g_stop_drawing = 0;

	g_kona_fb = fb;
	ret_val =
	    pi_mgr_dfs_add_request(&g_kona_fb->dfs_node, "lcd", PI_MGR_PI_ID_MM,
				   PI_OPP_ECONOMY);

	if (ret_val) {
		printk(KERN_ERR "Failed to add dfs request for LCD\n");
		ret = -EIO;
		goto fb_dfs_fail;
	}

#ifdef CONFIG_OF
	if (pdev->dev.of_node) {
		fb_data = get_of_data(pdev->dev.of_node);
		if (!fb_data)
			goto fb_data_failed;
		else /* Save the pointer needed in remove method */
			pdev->dev.platform_data = fb_data;
	} else {
#endif
		fb_data = pdev->dev.platform_data;
		if (!fb_data) {
			ret = -EINVAL;
			goto fb_data_failed;
		}
#ifdef CONFIG_OF
	}
#endif

	if (populate_dispdrv_cfg(fb, fb_data))
		goto dispdrv_data_failed;

	pr_err("Initialising in %s mode\n", fb->display_info->vmode ?
						"VIDEO" : "COMMAND");
	fb->display_ops = (DISPDRV_T *)DISP_DRV_GetFuncTable();

	spin_lock_init(&fb->lock);
	platform_set_drvdata(pdev, fb);
	fb->fb_data = fb_data;
	fb->pdev = pdev;

	mutex_init(&fb->update_sem);
	atomic_set(&fb->buff_idx, 0);
	atomic_set(&fb->is_fb_registered, 0);
	atomic_set(&fb->force_update, 0);
	init_completion(&fb->prev_buf_done_sem);
	init_completion(&vsync_event);
	complete(&fb->prev_buf_done_sem);
	atomic_set(&fb->is_graphics_started, 0);

	ret = enable_display(fb);
	if (ret) {
		konafb_error("Failed to enable this display device\n");
		goto err_enable_display_failed;
	} else {
		fb->is_display_found = 1;
	}

	framesize = fb->display_info->width * fb->display_info->height *
	    fb->display_info->Bpp;

	if (need_page_alignment)
		framesize = PAGE_ALIGN(framesize);
	framesize *= 2;

	/* Workaround: One page extra allocated and mapped via m4u to avoid
	 * v3d write faulting in m4u doing extra access */
	framesize_alloc = PAGE_ALIGN(framesize + 4096);
	fb->framesize_alloc = framesize_alloc;

	fb->fb.screen_base = dma_alloc_writecombine(&pdev->dev,
			framesize_alloc,
			&phys_fbbase,
			GFP_KERNEL);
	pr_info("kona_fb: screen_base=%p, phys_fbbase=%p size=0x%x\n",
			(void *)fb->fb.screen_base, (void *)phys_fbbase,
			framesize_alloc);
	if (fb->fb.screen_base == NULL) {
		ret = -ENOMEM;
		konafb_error("Unable to allocate fb memory\n");
		goto err_fbmem_alloc_failed;
	}
	dma_addr = phys_fbbase;
	direct_dma_addr = dma_addr;

#ifdef CONFIG_IOMMU_API
	pdev->dev.archdata.iommu = &fb_data->pdev_iommu->dev;
	pr_info("%s iommu-device(%p)\n", "framebuffer",
			pdev->dev.archdata.iommu);
#ifdef CONFIG_BCM_IOVMM
	need_map_switch = g_display_enabled && fb->display_info->vmode;
	if (need_map_switch) {
		/* 1:1 map for uboot logo */
		ret = kona_fb_direct_map(fb,
				framesize_alloc, phys_fbbase, direct_dma_addr);
		if (ret < 0) {
			pr_err("kona_fb_direct_map failed\n");
			goto err_set_var_failed;
		}
	}
	ret = kona_fb_iovmm_map(fb, framesize_alloc, phys_fbbase, &dma_addr);
#else
	ret = kona_fb_direct_map(fb,
			framesize_alloc, phys_fbbase, direct_dma_addr);
#endif /* CONFIG_BCM_IOVMM */
	pr_info("%16s: iommu map da(%#x) pa(%#x) size(%#x)\n",
			"framebuffer", dma_addr, phys_fbbase, framesize_alloc);
	if (ret < 0) {
		pr_err("kona_fb_iommu_mapping failed\n");
		goto err_set_var_failed;
	}
#endif /* CONFIG_IOMMU_API */

	/* Now we should get correct width and height for this display .. */
	width = fb->display_info->width;
	height = fb->display_info->height;
	fb->buff0 = (void *)dma_addr;
	fb->buff1 = (void *)dma_addr + framesize / 2;

	fb->fb.fbops = &kona_fb_ops;
	fb->fb.flags = FBINFO_FLAG_DEFAULT;
	fb->fb.pseudo_palette = fb->cmap;
	fb->fb.fix.type = FB_TYPE_PACKED_PIXELS;
	fb->fb.fix.visual = FB_VISUAL_TRUECOLOR;
	fb->fb.fix.line_length = width * fb->display_info->Bpp;
	fb->fb.fix.accel = FB_ACCEL_NONE;
	fb->fb.fix.ypanstep = 1;
	fb->fb.fix.xpanstep = 4;
#ifdef PARTIAL_UPDATE_SUPPORT
	fb->fb.fix.reserved[0] = 0x5444;
	fb->fb.fix.reserved[1] = 0x5055;
#endif
	fb->fb.var.xres = width;
	fb->fb.var.yres = height;
	fb->fb.var.xres_virtual = width;
	fb->fb.var.yres_virtual = height * 2;
	fb->fb.var.bits_per_pixel = fb->display_info->Bpp << 3;
	fb->fb.var.activate = FB_ACTIVATE_NOW;
	fb->fb.var.height = fb->display_info->phys_height;
	fb->fb.var.width = fb->display_info->phys_width;
	pixclock_64 = div_u64(1000000000000LLU,
				width * height * fb->display_info->fps);
	fb->fb.var.pixclock = pixclock_64;
	fb->fb.var.rotate = fb_data->rotation == 180 ?
						FB_ROTATE_UD : FB_ROTATE_UR;
	fb->rotate = fb->fb.var.rotate;

	switch (fb->display_info->in_fmt) {
	case DISPDRV_FB_FORMAT_RGB666P:
	case DISPDRV_FB_FORMAT_RGB666U:
	case DISPDRV_FB_FORMAT_RGB565:
		fb->fb.var.red.offset = 11;
		fb->fb.var.red.length = 5;
		fb->fb.var.green.offset = 5;
		fb->fb.var.green.length = 6;
		fb->fb.var.blue.offset = 0;
		fb->fb.var.blue.length = 5;
		break;

	case DISPDRV_FB_FORMAT_xRGB8888:
		fb->fb.var.transp.offset = 24;
		fb->fb.var.transp.length = 8;
		fb->fb.var.red.offset = 16;
		fb->fb.var.red.length = 8;
		fb->fb.var.green.offset = 8;
		fb->fb.var.green.length = 8;
		fb->fb.var.blue.offset = 0;
		fb->fb.var.blue.length = 8;
		break;

	case DISPDRV_FB_FORMAT_xBGR8888:
		fb->fb.var.transp.offset = 24;
		fb->fb.var.transp.length = 8;
		fb->fb.var.blue.offset = 16;
		fb->fb.var.blue.length = 8;
		fb->fb.var.green.offset = 8;
		fb->fb.var.green.length = 8;
		fb->fb.var.red.offset = 0;
		fb->fb.var.red.length = 8;
		break;

	default:
		konafb_error("Wrong format!\n");
		break;
	}

	fb->fb.fix.smem_start = phys_fbbase;
	fb->fb.fix.smem_len = framesize;

	konafb_debug(
		"Framebuffer starts at phys[0x%08x], da[0x%08x] and virt[0x%08x] with frame size[0x%08x]\n",
		 phys_fbbase, dma_addr, (uint32_t) fb->fb.screen_base,
		 framesize);

	ret = fb_set_var(&fb->fb, &fb->fb.var);
	if (ret) {
		konafb_error("fb_set_var failed\n");
		goto err_set_var_failed;
	}

	if (need_map_switch) {
		uboot_phys = readl(KONA_AXIPV_VA + REG_CUR_FRAME);
		uboot_kvirt = ioremap_nocache(uboot_phys, framesize / 2);
		pr_info("Copy uboot logo to fb, phys 0x%x kvirt 0x%x\n",
				uboot_phys, (uint32_t)uboot_kvirt);
		if (uboot_kvirt) {
			/* memcopy uboot buffer to kernel's buff1 */
			memcpy(fb->fb.screen_base + (framesize / 2),
				uboot_kvirt, framesize / 2);
			iounmap(uboot_kvirt);
			/* update display with 1:1 map */
			if (!fb->display_info->vmode)
				kona_clock_start(fb);
			if (fb->display_info->clear_panel_ram)
				(void)clear_panel_ram(fb);
			ret = fb->display_ops->update(fb->display_hdl,
				(void *)phys_fbbase +
				(framesize / 2), NULL, NULL);
			if (!fb->display_info->vmode)
				kona_clock_stop(fb);
			if (ret) {
				konafb_error("Can not enable the LCD!\n");
				goto err_fb_register_failed;
			}
			/* Wait new buffer. TODO: checking frame end */
			usleep_range(16666, 16668);
		}
	}

#ifdef CONFIG_IOMMU_API
	if (bcm_iommu_enable(&pdev->dev) < 0)
		konafb_error("bcm_iommu_enable failed\n");
#endif

	if (!fb->display_info->vmode)
		kona_clock_start(fb);
	/* Paint it black (assuming default fb contents are all zero) */
	/* Or Paint it with the logo uboot has initliased */
	if (g_display_enabled && !fb->display_info->vmode) {
		/* Keep the boot display for command display */
	} else {
		if (fb->display_info->clear_panel_ram)
			(void)clear_panel_ram(fb);
		else
			ret = fb->display_ops->update(fb->display_hdl,
						fb->buff1, NULL, NULL);
	}
	if (ret) {
		konafb_error("Can not enable the LCD!\n");
		/* Stop esc_clock in cmd mode since disable_display
		 * again starts clock for cmd mode */
		if (!fb->display_info->vmode)
			kona_clock_stop(fb);
		goto err_fb_register_failed;
	}

	/* Display on after painted blank */
	fb->display_ops->power_control(fb->display_hdl, CTRL_SCREEN_ON);
	if (!fb->display_info->vmode)
		kona_clock_stop(fb);

	if (need_map_switch) {
		/* To switch to new buffer. TODO: checking frame update end */
		usleep_range(16666, 16668);
#ifdef CONFIG_IOMMU_API
		/* unmap the uboot direct map */
		kona_fb_direct_unmap(fb, framesize_alloc, direct_dma_addr);
#endif
	}

	ret = register_framebuffer(&fb->fb);
	if (ret) {
		konafb_error("Framebuffer registration failed\n");
		goto err_fb_register_failed;
	}
#ifdef CONFIG_FRAMEBUFFER_FPS
	fb->fps_info = fb_fps_register(&fb->fb);
	if (NULL == fb->fps_info)
		printk(KERN_ERR "No fps display");
#endif

	atomic_set(&fb->is_fb_registered, 1);
	konafb_info("kona Framebuffer probe successfull\n");

#ifdef CONFIG_LOGO
	logo_rotate = fb->fb.var.rotate ? FB_ROTATE_UD : FB_ROTATE_UR;
	fb_prepare_logo(&fb->fb, logo_rotate);
	fb_show_logo(&fb->fb, logo_rotate);
	mutex_lock(&fb->update_sem);
	if (!fb->display_info->vmode) {
		kona_clock_start(fb);
		fb->display_ops->update(fb->display_hdl, fb->buff0, NULL, NULL);
		kona_clock_stop(fb);
	} else {
		fb->display_ops->update(fb->display_hdl, fb->buff0, NULL, NULL);
	}
	mutex_unlock(&fb->update_sem);
#endif

	if (fb->fb_data->esdcheck) {
		uint32_t tectl_gpio = fb->fb_data->tectl_gpio;
		konafb_info("Enable esd check function for lcd\n");
		if (tectl_gpio > 0) {
			konafb_info("Enable TECTL gpio check\n");
			gpio_request(tectl_gpio, "tectl_gpio");
			gpio_direction_input(tectl_gpio);
			ret = request_irq(gpio_to_irq(tectl_gpio),
					kona_fb_esd_irq, IRQF_TRIGGER_FALLING,
					"tectl_gpio", fb);
			if (ret < 0) {
				konafb_error("Request esd gpio irq fail\n");
				goto err_fb_register_failed;
			}
		}
		disable_irq(gpio_to_irq(tectl_gpio));
		init_completion(&fb->tectl_gpio_done_sem);

		fb->esd_check_wq =
			create_singlethread_workqueue("lcd_esd_check");
		INIT_DELAYED_WORK(&fb->esd_check_work, kona_fb_esd_check);
		queue_delayed_work(fb->esd_check_wq, &fb->esd_check_work,
			msecs_to_jiffies(fb->fb_data->esdcheck_period_ms));
	}


	fb->proc_entry = proc_create_data("fb_debug", 0666, NULL,
						&proc_fops, NULL);
	if (NULL == fb->proc_entry)
		printk(KERN_ERR "%s: could not create proc entry.\n", __func__);

	fb->reboot_nb.notifier_call = kona_fb_reboot_cb;
	register_reboot_notifier(&fb->reboot_nb);
#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
	fb->die_nb.notifier_call = kona_fb_die_cb;
	register_die_notifier(&fb->die_nb);
#endif
	ret = register_attributes(&pdev->dev);
	if (ret)
		dev_err(&pdev->dev, "%s: failed to register attributes\n",
								__func__);
#ifdef CONFIG_DEBUG_FS
	kona_fb_create_debugfs(pdev);
#endif

	pr_info("%s(%d): Probe success. Panel: %s\n",
					__func__, __LINE__, fb->fb_data->name);
	return 0;

err_fb_register_failed:
err_set_var_failed:
	dma_free_writecombine(&pdev->dev, framesize_alloc, fb->fb.screen_base,
			phys_fbbase);
err_fbmem_alloc_failed:
	disable_display(fb);
err_enable_display_failed:
	release_dispdrv_info(fb->display_info);
dispdrv_data_failed:
	free_platform_data(&pdev->dev);
fb_data_failed:
	if (pi_mgr_dfs_request_remove(&fb->dfs_node))
		printk(KERN_ERR "Failed to remove dfs request for LCD\n");
fb_dfs_fail:
	kfree(fb);
	g_kona_fb = NULL;
err_fb_alloc_failed:
	pr_err("%s failed ret=%d\n", __func__, ret);
	return ret;
}

static int kona_fb_reboot_cb(struct notifier_block *nb,
	unsigned long val, void *v)
{
	struct kona_fb *fb = container_of(nb, struct kona_fb, reboot_nb);
	struct fb_event event;
	int blank = FB_BLANK_POWERDOWN;

	pr_err("Turning off display\n");
	if (fb->g_stop_drawing) {
		pr_err("Display is already suspended, nothing to do\n");
		goto exit;
	}

	/*shut down the backlight before disable the display*/
	pr_info("Turning off backlight\r\n");
	event.info = &fb->fb;
	event.data = &blank;
	fb_notifier_call_chain(FB_EVENT_BLANK, &event);

	mutex_lock(&fb->update_sem);
	if (fb->blank_state == KONA_FB_LINK_SUSPENDED) {
		konafb_debug("Resume link\n");
		link_control(fb, RESUME_LINK);
	}

	fb->g_stop_drawing = 1;
	if (!fb->display_info->vmode) {
		if (wait_for_completion_timeout(
		&fb->prev_buf_done_sem,	msecs_to_jiffies(10000)) <= 0)
			pr_err("%s:%d timed out waiting for completion",
				__func__, __LINE__);
		kona_clock_start(fb);
		fb->display_ops->power_control(
					fb->display_hdl, CTRL_SCREEN_OFF);
		kona_clock_stop(fb);
	} else {
		fb->display_ops->power_control(
					fb->display_hdl, CTRL_SCREEN_OFF);
	}
	disable_display(fb);
	mutex_unlock(&fb->update_sem);
	pr_err("Display disabled\n");
exit:
	return 0;
}

#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
static int kona_fb_die_cb(struct notifier_block *nb, unsigned long val, void *v)
{
	pr_err("kona_fb: die notifier invoked\n");
	if (!crash_dump_ui_on) {
		if (ramdump_enable)
			kona_display_crash_image(AP_RAM_DUMP_START);
		else
			kona_display_crash_image(AP_CRASH_DUMP_START);
		crash_dump_ui_on = 1;
	}
	return NOTIFY_DONE;
}
#endif

static int kona_fb_remove(struct platform_device *pdev)
{
	struct kona_fb *fb = platform_get_drvdata(pdev);
	struct kona_fb_platform_data *pdata = (struct kona_fb_platform_data *)
						pdev->dev.platform_data;

	remove_attributes(&pdev->dev);
#ifdef CONFIG_DEBUG_FS
	kona_fb_remove_debugfs(pdev);
#endif
#ifdef CONFIG_FRAMEBUFFER_FPS
	fb_fps_unregister(fb->fps_info);
#endif
	unregister_framebuffer(&fb->fb);
	disable_display(fb);
#ifdef CONFIG_IOMMU_API
#ifdef CONFIG_BCM_IOVMM
	kona_fb_iovmm_unmap(fb, fb->framesize_alloc, (dma_addr_t)fb->buff0);
#else
	kona_fb_direct_unmap(fb, fb->framesize_alloc, (dma_addr_t)fb->buff0);
#endif
#endif
	if (pdev->dev.of_node) {
		kfree(pdata);
		pdev->dev.platform_data = NULL;
	}

	kfree(fb);
	konafb_info("kona FB removed !!\n");
	return 0;
}

static const struct of_device_id kona_fb_of_match[] = {
	{ .compatible = "bcm,kona-fb", },
	{},
};
MODULE_DEVICE_TABLE(of, kona_fb_of_match);


static struct platform_driver kona_fb_driver = {
	.probe = kona_fb_probe,
	.remove = kona_fb_remove,
	.driver = {
		   .name = "kona_fb",
		   .owner = THIS_MODULE,
		   .of_match_table = kona_fb_of_match,
		   },
};

static int __init kona_fb_init(void)
{
	int ret;

	ret =
	    pi_mgr_qos_add_request(&g_mm_qos_node, "lcd", PI_MGR_PI_ID_MM, 10);
	if (ret)
		printk(KERN_ERR "failed to register qos client for lcd\n");

	ret = platform_driver_register(&kona_fb_driver);
	if (ret) {
		printk(KERN_ERR
		       "%s : Unable to register kona framebuffer driver\n",
		       __func__);
		goto fail_to_register;
	}
fail_to_register:
	printk(KERN_INFO "BRCM Framebuffer Init %s !\n", ret ? "FAILED" : "OK");

	return ret;
}

static void __exit kona_fb_exit(void)
{
	platform_driver_unregister(&kona_fb_driver);
	printk(KERN_INFO "BRCM Framebuffer exit OK\n");
}

module_init(kona_fb_init);
module_exit(kona_fb_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("KONA FB Driver");
