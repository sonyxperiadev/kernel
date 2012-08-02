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
#include <video/kona_fb_boot.h>
#include <video/kona_fb.h>
#include <mach/io.h>
#ifdef CONFIG_FRAMEBUFFER_FPS
#include <linux/fb_fps.h>
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/clk.h>
#include <plat/pi_mgr.h>
#include <linux/broadcom/mobcom_types.h>

#include "kona_fb.h"
#include "lcd/display_drv.h"

#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
#include <video/kona_fb_image_dump.h>
#include "lcd/cp_crash_start_565.h"
#include "lcd/cp_crash_end_565.h"
#endif

/*#define kona_FB_DEBUG */
/*#define PARTIAL_UPDATE_SUPPORT */
#define kona_FB_ENABLE_DYNAMIC_CLOCK	1

#define kona_IOCTL_SET_BUFFER_AND_UPDATE	_IO('F', 0x80)

static struct pi_mgr_qos_node g_mm_qos_node;

#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
static DEFINE_SPINLOCK(g_fb_crash_spin_lock);
#endif

struct kona_fb {
	dma_addr_t phys_fbbase;
	spinlock_t lock;
	struct task_struct *thread;
	struct completion thread_sem;
	struct mutex update_sem;
	struct completion prev_buf_done_sem;
	atomic_t buff_idx;
	atomic_t is_fb_registered;
	atomic_t is_graphics_started;
	int base_update_count;
	int rotation;
	int is_display_found;
#ifdef CONFIG_FRAMEBUFFER_FPS
	struct fb_fps_info *fps_info;
#endif
	struct fb_info fb;
	u32 cmap[16];
	DISPDRV_T *display_ops;
	const DISPDRV_INFO_T *display_info;
	DISPDRV_HANDLE_T display_hdl;
	struct pi_mgr_dfs_node dfs_node;
	int g_stop_drawing;
	u32 gpio;
	u32 bus_width;
	struct proc_dir_entry *proc_entry;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend_level1;
	struct early_suspend early_suspend_level2;
	struct early_suspend early_suspend_level3;
#endif
	void *buff0;
	void *buff1;
	atomic_t force_update;
	struct dispdrv_init_parms lcd_drv_parms;
};

static struct kona_fb *g_kona_fb;

#ifdef CONFIG_FB_BRCM_ASYNC_UPDATE
static int g_vsync_cnt;
static volatile struct timeval prev_timeval, curr_timeval;
#endif

#ifdef kona_FB_DEBUG
#define kona_PROF_N_RECORDS 50
static volatile struct {
	struct timeval	curr_time;
	struct timeval	prev_time;
	int is_late;
	int use_te;
} kona_fb_profile[kona_PROF_N_RECORDS];

static volatile u32 kona_fb_profile_cnt;

void kona_fb_profile_record(struct timeval prev_timeval,
		struct timeval curr_timeval, int is_too_late, int do_vsync)
{
	kona_fb_profile[kona_fb_profile_cnt].curr_time =  curr_timeval;
	kona_fb_profile[kona_fb_profile_cnt].prev_time =  prev_timeval;
	kona_fb_profile[kona_fb_profile_cnt].is_late   = is_too_late;
	kona_fb_profile[kona_fb_profile_cnt].use_te   = do_vsync;
	kona_fb_profile_cnt = (kona_fb_profile_cnt+1) % kona_PROF_N_RECORDS;
}

static int
proc_write_fb_test(struct file *file, const char __user *buffer,
			unsigned long count, void *data)
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
	for (cnt = 0; cnt < kona_PROF_N_RECORDS; cnt++) {
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
		i = (i + 1) % kona_PROF_N_RECORDS;
	}

	return len;
}
#else
#define kona_fb_profile_record(prev_timeval, curr_timeval, is_too_late, \
		do_vsync) do { } while (0)
#define proc_write_fb_test NULL
#endif /* kona_FB_DEBUG */

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
	struct kona_fb *fb = container_of(info, struct kona_fb, fb);

	konafb_debug("kona %s\n", __func__);

	if (fb->rotation != fb->fb.var.rotate) {
		konafb_warning("Rotation is not supported yet !\n");
		return -EINVAL;
	}

	return 0;
}

static inline void kona_clock_start(struct kona_fb *fb)
{
#if (kona_FB_ENABLE_DYNAMIC_CLOCK == 1)
	fb->display_ops->start(fb->display_hdl, &fb->dfs_node);
#endif
}

static inline void kona_clock_stop(struct kona_fb *fb)
{
#if (kona_FB_ENABLE_DYNAMIC_CLOCK == 1)
	fb->display_ops->stop(fb->display_hdl, &fb->dfs_node);
#endif
}


#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
void kona_display_crash_image(enum crash_dump_image_idx image_idx)
{
	int ret = 0;
	unsigned long flags, image_size, pos, count, index;
	u16 *image_buf, *lcd_buf, len, bytes_565;

	atomic_set(&g_kona_fb->force_update, 1);
	spin_lock_irqsave(&g_fb_crash_spin_lock, flags);
	kona_clock_start(g_kona_fb);

	if (image_idx == CP_CRASH_DUMP_START) {
		image_buf = (u16 *)&cp_crash_start_565_rle[0];
		image_size = sizeof(cp_crash_start_565_rle);
	} else {
		image_buf = (u16 *)&cp_crash_end_565_rle[0];
		image_size = sizeof(cp_crash_end_565_rle);
	}

	lcd_buf   = (u16 *)g_kona_fb->fb.screen_base;
	pos = 0;
	if (image_size % 4)
		printk(KERN_ERR "Wrong image source!");
	for (count = 0; count < image_size/2; count += 2) {
		len = image_buf[count];
		bytes_565 = image_buf[count+1];
		for (index = 0; index < len; index++) {
			lcd_buf[pos++] = bytes_565;
			if (pos >  g_kona_fb->fb.fix.smem_len / 4)
				printk(KERN_ERR "Wrong image size!");
		}
	}

	if (g_kona_fb->display_ops->update_no_os) {
		ret =
		g_kona_fb->display_ops->update_no_os(g_kona_fb->display_hdl,
				g_kona_fb->buff0,
				NULL);
	}

	g_kona_fb->g_stop_drawing = 1;
	spin_unlock_irqrestore(&g_fb_crash_spin_lock, flags);
}
#endif

static void kona_display_done_cb(int status)
{
	(void)status;
	kona_clock_stop(g_kona_fb);
#ifdef CONFIG_FB_BRCM_ASYNC_UPDATE
	do_gettimeofday((struct timeval *)&prev_timeval);
#endif
	complete(&g_kona_fb->prev_buf_done_sem);
}

static int kona_fb_pan_display(struct fb_var_screeninfo *var,
			       struct fb_info *info)
{
	int ret = 0;
#ifdef CONFIG_FB_BRCM_ASYNC_UPDATE
	int is_too_late;
	s64 frame_gap;
	static int do_vsync;
#endif
	struct kona_fb *fb = container_of(info, struct kona_fb, fb);
	uint32_t buff_idx;
#ifdef CONFIG_FRAMEBUFFER_FPS
	void *dst;
#endif
	DISPDRV_WIN_t region, *p_region;

	buff_idx = var->yoffset ? 1 : 0;

	konafb_debug("kona %s with buff_idx =%d \n", __func__, buff_idx);

	if (mutex_lock_killable(&fb->update_sem))
		return -EINTR;

	if (1 == fb->g_stop_drawing) {
		konafb_debug
		    ("kona FB/LCd is in the early suspend state and stops drawing now!");
		goto skip_drawing;
	}

	atomic_set(&fb->buff_idx, buff_idx);

#ifdef CONFIG_FRAMEBUFFER_FPS
	dst = (fb->fb.screen_base) +
	    (buff_idx * fb->fb.var.xres * fb->fb.var.yres *
	     (fb->fb.var.bits_per_pixel / 8));
	fb_fps_display(fb->fps_info, dst, 5, 2, 0);
#endif

	if (!atomic_read(&fb->is_fb_registered)) {
		kona_clock_start(fb);
		ret =
		    fb->display_ops->update(fb->display_hdl,
					    buff_idx ? fb->buff1 : fb->buff0,
					    NULL, NULL);
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
		wait_for_completion(&fb->prev_buf_done_sem);

#ifdef CONFIG_FB_BRCM_ASYNC_UPDATE
		g_vsync_cnt++;
		do_gettimeofday((struct timeval *)&curr_timeval);
		frame_gap = (s64)(curr_timeval.tv_sec - prev_timeval.tv_sec) *
			1000000 + (s64)(curr_timeval.tv_usec -
					prev_timeval.tv_usec);
		if (frame_gap > 1000)
			is_too_late = 1;
		else
			is_too_late = 0;

		if ((g_vsync_cnt == 1) || (!do_vsync) || is_too_late)
			do_vsync = 1;
		else
			do_vsync = 0;

		kona_fb_profile_record(prev_timeval, curr_timeval,
					is_too_late, do_vsync);

		kona_clock_start(fb);
		ret =
		    fb->display_ops->update(fb->display_hdl,
					buff_idx ? fb->buff1 : fb->buff0,
					(DISPDRV_WIN_t *)do_vsync,
					(DISPDRV_CB_T)kona_display_done_cb);
#else
		kona_clock_start(fb);
		ret =
		    fb->display_ops->update(fb->display_hdl,
					buff_idx ? fb->buff1 : fb->buff0,
					p_region,
					(DISPDRV_CB_T)kona_display_done_cb);

#endif /* CONFIG_FB_BRCM_ASYNC_UPDATE */
	}
skip_drawing:
	mutex_unlock(&fb->update_sem);

	konafb_debug
	    ("kona Display is updated once at %d time with yoffset=%d\n",
	     fb->base_update_count, var->yoffset);
	return ret;
}

static int enable_display(struct kona_fb *fb, struct dispdrv_init_parms *parms)
{
	int ret = 0;

	ret = fb->display_ops->init(parms, &fb->display_hdl);
	if (ret != 0) {
		konafb_error("Failed to init this display device!\n");
		goto fail_to_init;
	}

	fb->display_info = fb->display_ops->get_info(fb->display_hdl);

	kona_clock_start(fb);
	ret = fb->display_ops->open(fb->display_hdl);
	if (ret != 0) {
		konafb_error("Failed to open this display device!\n");
		goto fail_to_open;
	}

	ret = fb->display_ops->power_control(fb->display_hdl, CTRL_PWR_ON);
	if (ret != 0) {
		konafb_error("Failed to power on this display device!\n");
		goto fail_to_power_control;
	}

	kona_clock_stop(fb);
	konafb_info("kona display is enabled successfully\n");
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

	fb->display_ops->power_control(fb->display_hdl, CTRL_PWR_OFF);
	fb->display_ops->close(fb->display_hdl);
	fb->display_ops->exit(fb->display_hdl);

	konafb_info("kona display is disabled successfully\n");
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

	case kona_IOCTL_SET_BUFFER_AND_UPDATE:

		if (mutex_lock_killable(&fb->update_sem))
			return -EINTR;
		ptr = (void *)arg;

		if (ptr == NULL) {
			mutex_unlock(&fb->update_sem);
			return -EFAULT;
		}

		wait_for_completion(&fb->prev_buf_done_sem);
		kona_clock_start(fb);
		ret = fb->display_ops->update(fb->display_hdl, ptr, NULL, NULL);
		kona_clock_stop(fb);
		complete(&g_kona_fb->prev_buf_done_sem);
		mutex_unlock(&fb->update_sem);
		break;

	default:

		konafb_error("Wrong ioctl cmd\n");
		break;
	}

	return ret;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void kona_fb_early_suspend(struct early_suspend *h)
{
	struct kona_fb *fb;

	konafb_error("BRCM fb early suspend with level = %d\n", h->level);

	switch (h->level) {

	case EARLY_SUSPEND_LEVEL_BLANK_SCREEN:
		/* Turn off the backlight */
		fb = container_of(h, struct kona_fb, early_suspend_level1);
		mutex_lock(&fb->update_sem);
		wait_for_completion(&fb->prev_buf_done_sem);
		kona_clock_start(fb);
		if (fb->display_ops->power_control(fb->display_hdl,
					       CTRL_SCREEN_OFF))
			konafb_error("Failed to blank this display device!\n");
		kona_clock_stop(fb);
		complete(&g_kona_fb->prev_buf_done_sem);
		mutex_unlock(&fb->update_sem);

		break;

	case EARLY_SUSPEND_LEVEL_STOP_DRAWING:
		fb = container_of(h, struct kona_fb, early_suspend_level2);
		mutex_lock(&fb->update_sem);
		wait_for_completion(&fb->prev_buf_done_sem);
		fb->g_stop_drawing = 1;
		complete(&g_kona_fb->prev_buf_done_sem);
		mutex_unlock(&fb->update_sem);
		break;

	case EARLY_SUSPEND_LEVEL_DISABLE_FB:
		fb = container_of(h, struct kona_fb, early_suspend_level3);
		/* screen goes to sleep mode */
		mutex_lock(&fb->update_sem);
		kona_clock_start(fb);
		disable_display(fb);
		kona_clock_stop(fb);
		mutex_unlock(&fb->update_sem);
		/* Ok for MM going to shutdown state */
		pi_mgr_qos_request_update(&g_mm_qos_node,
					  PI_MGR_QOS_DEFAULT_VALUE);
		break;

	default:
		konafb_error("Early suspend with the wrong level!\n");
		break;
	}
}

static void kona_fb_late_resume(struct early_suspend *h)
{
	struct kona_fb *fb;

	konafb_error("BRCM fb late resume with level = %d\n", h->level);

	switch (h->level) {

	case EARLY_SUSPEND_LEVEL_BLANK_SCREEN:
		/* Turn on the backlight */
		fb = container_of(h, struct kona_fb, early_suspend_level1);
		kona_clock_start(fb);
		if (fb->display_ops->
		    power_control(fb->display_hdl, CTRL_SCREEN_ON))
			konafb_error
			    ("Failed to unblank this display device!\n");
		kona_clock_stop(fb);
#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
		if (atomic_read(&g_kona_fb->force_update))
			kona_display_crash_image(CP_CRASH_DUMP_START);
#endif
		break;

	case EARLY_SUSPEND_LEVEL_STOP_DRAWING:
		fb = container_of(h, struct kona_fb, early_suspend_level2);
		mutex_lock(&fb->update_sem);
		fb->g_stop_drawing = 0;
		mutex_unlock(&fb->update_sem);
		break;

	case EARLY_SUSPEND_LEVEL_DISABLE_FB:
		fb = container_of(h, struct kona_fb, early_suspend_level3);
		/* Ok for MM going to retention but not shutdown state */
		pi_mgr_qos_request_update(&g_mm_qos_node, 10);
		/* screen comes out of sleep */
		if (enable_display(fb, &fb->lcd_drv_parms))
			konafb_error("Failed to enable this display device\n");
		break;

	default:
		konafb_error("Early suspend with the wrong level!\n");
		break;
	}

}
#endif

static struct fb_ops kona_fb_ops = {
	.owner = THIS_MODULE,
	.fb_check_var = kona_fb_check_var,
	.fb_set_par = kona_fb_set_par,
	.fb_setcolreg = kona_fb_setcolreg,
	.fb_pan_display = kona_fb_pan_display,
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
	.fb_ioctl = kona_fb_ioctl,
};

static int kona_fb_probe(struct platform_device *pdev)
{
	int ret = -ENXIO;
	struct kona_fb *fb;
	size_t framesize;
	uint32_t width, height;
	int ret_val = -1;
	struct kona_fb_platform_data *fb_data;

printk("%s:%d\n", __func__, __LINE__);

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
				   PI_MGR_DFS_MIN_VALUE);
	if (ret_val) {
		printk(KERN_ERR "Failed to add dfs request for LCD\n");
		ret = -EIO;
		goto fb_data_failed;
	}

	fb_data = pdev->dev.platform_data;
	if (!fb_data) {
		ret = -EINVAL;
		goto fb_data_failed;
	}
	fb->display_ops = (DISPDRV_T *) fb_data->dispdrv_entry();

	spin_lock_init(&fb->lock);
	platform_set_drvdata(pdev, fb);

	mutex_init(&fb->update_sem);
	atomic_set(&fb->buff_idx, 0);
	atomic_set(&fb->is_fb_registered, 0);
	atomic_set(&fb->force_update, 0);
	init_completion(&fb->prev_buf_done_sem);
	complete(&fb->prev_buf_done_sem);
	atomic_set(&fb->is_graphics_started, 0);
	init_completion(&fb->thread_sem);

#if (kona_FB_ENABLE_DYNAMIC_CLOCK != 1)
	fb->display_ops->start(&fb->dfs_node);
#endif
	/* Enable_display will start/stop clocks on its own if dynamic */
	fb->lcd_drv_parms = *(struct dispdrv_init_parms *)&fb_data->parms;
	ret = enable_display(fb, &fb->lcd_drv_parms);
	if (ret) {
		konafb_error("Failed to enable this display device\n");
		goto err_enable_display_failed;
	} else {
		fb->is_display_found = 1;
	}

	framesize = fb->display_info->width * fb->display_info->height *
	    fb->display_info->Bpp * 2;
	fb->fb.screen_base = dma_alloc_writecombine(&pdev->dev,
						    framesize, &fb->phys_fbbase,
						    GFP_KERNEL);
#if 1
	/* Test pattern */
	char *dest = fb->fb.screen_base;
	int i, j;
	for (i = 0, j = 1; i < framesize;) {
		dest[i] = j+2;
		dest[i+1] = j+1;
		dest[i+2] = j;
		dest[i+3] = 0;
		i += 4;
		j += 3;
	}
#endif

	if (fb->fb.screen_base == NULL) {
		ret = -ENOMEM;
		konafb_error("Unable to allocate fb memory\n");
		goto err_fbmem_alloc_failed;
	}

	/* Now we should get correct width and height for this display .. */
	width = fb->display_info->width;
	height = fb->display_info->height;
	fb->buff0 = (void *)fb->phys_fbbase;
	fb->buff1 =
	    (void *)fb->phys_fbbase + width * height * fb->display_info->Bpp;

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

	switch (fb->display_info->input_format) {
	case DISPDRV_FB_FORMAT_RGB565:
		fb->fb.var.red.offset = 11;
		fb->fb.var.red.length = 5;
		fb->fb.var.green.offset = 5;
		fb->fb.var.green.length = 6;
		fb->fb.var.blue.offset = 0;
		fb->fb.var.blue.length = 5;
		framesize = width * height * 2 * 2;
		break;

	case DISPDRV_FB_FORMAT_RGB888_U:
		fb->fb.var.red.offset = 16;
		fb->fb.var.red.length = 8;
		fb->fb.var.green.offset = 8;
		fb->fb.var.green.length = 8;
		fb->fb.var.blue.offset = 0;
		fb->fb.var.blue.length = 8;
		fb->fb.var.transp.offset = 24;
		fb->fb.var.transp.length = 8;
		framesize = width * height * 4 * 2;
		break;

	default:
		konafb_error("Wrong format!\n");
		break;
	}

	fb->fb.fix.smem_start = fb->phys_fbbase;
	fb->fb.fix.smem_len = framesize;

printk("%s:%d\n", __func__, __LINE__);
	konafb_debug
	    ("Framebuffer starts at phys[0x%08x], and virt[0x%08x] with frame size[0x%08x]\n",
	     fb->phys_fbbase, (uint32_t) fb->fb.screen_base, framesize);

	ret = fb_set_var(&fb->fb, &fb->fb.var);
	if (ret) {
		konafb_error("fb_set_var failed\n");
		goto err_set_var_failed;
	}
	/* Paint it black (assuming default fb contents are all zero) */
	ret = kona_fb_pan_display(&fb->fb.var, &fb->fb);

#if 1
	ret = 0;
#endif

	if (ret) {
		konafb_error("Can not enable the LCD!\n");
		goto err_fb_register_failed;
	}

	if (fb->display_ops->set_brightness) {
		kona_clock_start(fb);
		fb->display_ops->set_brightness(fb->display_hdl, 100);
		kona_clock_stop(fb);
	}

	/* Display on after painted blank */
	kona_clock_start(fb);
	fb->display_ops->power_control(fb->display_hdl, CTRL_SCREEN_ON);
	kona_clock_stop(fb);

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
	complete(&fb->thread_sem);

	atomic_set(&fb->is_fb_registered, 1);
	konafb_info("kona Framebuffer probe successfull\n");

#ifdef CONFIG_LOGO
	fb_prepare_logo(&fb->fb, 0);
	fb_show_logo(&fb->fb, 0);

	mutex_lock(&fb->update_sem);
	kona_clock_start(fb);
	fb->display_ops->update(fb->display_hdl, fb->buff0, NULL, NULL);
	kona_clock_stop(fb);
	mutex_unlock(&fb->update_sem);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	fb->early_suspend_level1.suspend = kona_fb_early_suspend;
	fb->early_suspend_level1.resume = kona_fb_late_resume;
	fb->early_suspend_level1.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	register_early_suspend(&fb->early_suspend_level1);

	fb->early_suspend_level2.suspend = kona_fb_early_suspend;
	fb->early_suspend_level2.resume = kona_fb_late_resume;
	fb->early_suspend_level2.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING;
	register_early_suspend(&fb->early_suspend_level2);

	fb->early_suspend_level3.suspend = kona_fb_early_suspend;
	fb->early_suspend_level3.resume = kona_fb_late_resume;
	fb->early_suspend_level3.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
	register_early_suspend(&fb->early_suspend_level3);
#endif

	fb->proc_entry = create_proc_entry("fb_debug", 0666, NULL);

	if (NULL == fb->proc_entry)
		printk(KERN_ERR "%s: could not create proc entry.\n", __func__);
	else {
		fb->proc_entry->data = NULL;
		fb->proc_entry->read_proc = NULL;
		fb->proc_entry->write_proc = proc_write_fb_test;
	}

	return 0;

err_fb_register_failed:
err_set_var_failed:
	dma_free_writecombine(&pdev->dev, fb->fb.fix.smem_len,
			      fb->fb.screen_base, fb->fb.fix.smem_start);

	kona_clock_start(fb);
	disable_display(fb);
	kona_clock_stop(fb);

err_enable_display_failed:
#if (kona_FB_ENABLE_DYNAMIC_CLOCK != 1)
	fb->display_ops->stop(&fb->dfs_node);
#endif
err_fbmem_alloc_failed:
	if (pi_mgr_dfs_request_remove(&fb->dfs_node))
		printk(KERN_ERR "Failed to remove dfs request for LCD\n");
fb_data_failed:
	kfree(fb);
	g_kona_fb = NULL;
err_fb_alloc_failed:
	return ret;
}

static int __devexit kona_fb_remove(struct platform_device *pdev)
{
	size_t framesize;
	struct kona_fb *fb = platform_get_drvdata(pdev);

	framesize = fb->fb.var.xres_virtual * fb->fb.var.yres_virtual * 2;

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&fb->early_suspend_level1);
	unregister_early_suspend(&fb->early_suspend_level2);
	unregister_early_suspend(&fb->early_suspend_level3);
#endif

#ifdef CONFIG_FRAMEBUFFER_FPS
	fb_fps_unregister(fb->fps_info);
#endif
	unregister_framebuffer(&fb->fb);
	disable_display(fb);
	kfree(fb);
	konafb_info("kona FB removed !!\n");
	return 0;
}

static struct platform_driver kona_fb_driver = {
	.probe = kona_fb_probe,
	.remove = __devexit_p(kona_fb_remove),
	.driver = {
		   .name = "kona_fb"}
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

late_initcall(kona_fb_init);
module_exit(kona_fb_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("kona FB Driver");
