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

#ifndef KONAFB_OLD_API
#include <video/kona_fb_boot.h>
#endif

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

#include "rhea_fb.h"
#include "lcd/display_drv.h"

//#define RHEA_FB_DEBUG 
//#define PARTIAL_UPDATE_SUPPORT
#define RHEA_FB_ENABLE_DYNAMIC_CLOCK	1

#define RHEA_IOCTL_SET_BUFFER_AND_UPDATE	_IO('F', 0x80)

#define SHARP_RESET  	 (13)     
#define DSI_BRIDGE_RESET (12)     
#define DSI_BRIDGE_PON   (25)

static struct pi_mgr_qos_node g_mm_qos_node;

#ifdef CONFIG_CDEBUGGER
struct struct_frame_buf_mark {
	u32 special_mark_1;
	u32 special_mark_2;
	u32 special_mark_3;
	u32 special_mark_4;
	void *p_fb; /* it must be physical address */
	u32 resX;
	u32 resY;
	u32 bpp;    /* color depth : 16 or 24 */
	u32 frames; /* frame buffer count : 2 */
};

static struct struct_frame_buf_mark  frame_buf_mark = {
	.special_mark_1 = (('*' << 24) | ('^' << 16) | ('^' << 8) | ('*' << 0)),
	.special_mark_2 = (('I' << 24) | ('n' << 16) | ('f' << 8) | ('o' << 0)),
	.special_mark_3 = (('H' << 24) | ('e' << 16) | ('r' << 8) | ('e' << 0)),
	.special_mark_4 = (('f' << 24) | ('b' << 16) | ('u' << 8) | ('f' << 0)),
	.p_fb   = 0,
	.frames = 1
};
#endif

struct rhea_fb {
	dma_addr_t phys_fbbase;
	spinlock_t lock;
	struct task_struct *thread;
	struct completion thread_sem;
	struct mutex update_sem;
	struct completion prev_buf_done_sem;
#if !defined(CONFIG_MACH_RHEA_RAY_EDN1X) && !defined(CONFIG_MACH_RHEA_BERRI) && !defined(CONFIG_MACH_RHEA_RAY_EDN2X) && !defined(CONFIG_MACH_RHEA_SS_COMMON)  \
	&& !defined(CONFIG_MACH_RHEA_RAY_DEMO) && !defined(CONFIG_MACH_RHEA_BERRI_EDN40) && !defined(CONFIG_MACH_RHEA_STONE)
	struct completion refresh_wait_sem;
#endif
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
	u32	cmap[16];
	DISPDRV_T *display_ops;
	const DISPDRV_INFO_T *display_info;
	DISPDRV_HANDLE_T display_hdl; 
	struct pi_mgr_dfs_node dfs_node;
	int g_stop_drawing; 
	u32 gpio;
	u32 bus_width;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend_level1;
	struct early_suspend early_suspend_level2;
	struct early_suspend early_suspend_level3;
#endif
#ifndef KONAFB_OLD_API
	void *buff0;
	void *buff1;
	struct dispdrv_init_parms lcd_drv_parms;
#endif
};

static struct rhea_fb *g_rhea_fb = NULL;

static inline u32 convert_bitfield(int val, struct fb_bitfield *bf)
{
	unsigned int mask = (1 << bf->length) - 1;

	return (val >> (16 - bf->length) & mask) << bf->offset;
}

static int
rhea_fb_setcolreg(unsigned int regno, unsigned int red, unsigned int green,
		 unsigned int blue, unsigned int transp, struct fb_info *info)
{
	struct rhea_fb *fb = container_of(info, struct rhea_fb, fb);

	rheafb_debug("RHEA regno = %d r=%d g=%d b=%d\n", regno, red, green, blue);

	if (regno < 16) {
		fb->cmap[regno] = convert_bitfield(transp, &fb->fb.var.transp) |
				  convert_bitfield(blue, &fb->fb.var.blue) |
				  convert_bitfield(green, &fb->fb.var.green) |
				  convert_bitfield(red, &fb->fb.var.red);
		return 0;
	}
	else {
		return 1;
	}
}

static int rhea_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	rheafb_debug("RHEA %s\n", __func__);

	if((var->rotate & 1) != (info->var.rotate & 1)) {
		if((var->xres != info->var.yres) ||
		   (var->yres != info->var.xres) ||
		   (var->xres_virtual != info->var.yres) ||
		   (var->yres_virtual > info->var.xres * 2) ||
		   (var->yres_virtual < info->var.xres )) {
			rheafb_error("fb_check_var_failed\n");
			return -EINVAL;
		}
	} else {
		if((var->xres != info->var.xres) ||
		   (var->yres != info->var.yres) ||
		   (var->xres_virtual != info->var.xres) ||
		   (var->yres_virtual > info->var.yres * 2) ||
		   (var->yres_virtual < info->var.yres )) {
			rheafb_error("fb_check_var_failed\n");
			return -EINVAL;
		}
	}

	return 0;
}

static int rhea_fb_set_par(struct fb_info *info)
{
	struct rhea_fb *fb = container_of(info, struct rhea_fb, fb);

	rheafb_debug("RHEA %s\n", __func__);

	if(fb->rotation != fb->fb.var.rotate) {
		rheafb_warning("Rotation is not supported yet !\n");
		return -EINVAL;
	}

	return 0;
}

static inline void rhea_clock_start(struct rhea_fb *fb)
{
#if (RHEA_FB_ENABLE_DYNAMIC_CLOCK == 1)
	fb->display_ops->start(fb->display_hdl, &fb->dfs_node);
#endif
}

static inline void rhea_clock_stop(struct rhea_fb *fb)
{
#if (RHEA_FB_ENABLE_DYNAMIC_CLOCK == 1)
	fb->display_ops->stop(fb->display_hdl, &fb->dfs_node);
#endif
}

static void rhea_display_done_cb(int status)
{	
	(void)status;
	rhea_clock_stop(g_rhea_fb);
	complete(&g_rhea_fb->prev_buf_done_sem);
}

static int rhea_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	int ret = 0;
	struct rhea_fb *fb = container_of(info, struct rhea_fb, fb);
	uint32_t buff_idx;
#ifdef CONFIG_FRAMEBUFFER_FPS
	void *dst;
#endif
        DISPDRV_WIN_t region, *p_region;

	buff_idx = var->yoffset ? 1 : 0;

	rheafb_debug("RHEA %s with buff_idx =%d \n", __func__, buff_idx);

	if (mutex_lock_killable(&fb->update_sem))
		return -EINTR;

	if (1 == fb->g_stop_drawing) {
		rheafb_debug("RHEA FB/LCd is in the early suspend state and stops drawing now!");
		goto skip_drawing;
	}

	atomic_set(&fb->buff_idx, buff_idx);

#ifdef CONFIG_FRAMEBUFFER_FPS
	dst = (fb->fb.screen_base) + 
		(buff_idx * fb->fb.var.xres * fb->fb.var.yres * (fb->fb.var.bits_per_pixel/8));
	fb_fps_display(fb->fps_info, dst, 5, 2, 0);
#endif
	
	if (!atomic_read(&fb->is_fb_registered)) {
		rhea_clock_start(fb);
//		ret = fb->display_ops->update(fb->display_hdl, buff_idx, NULL, NULL /* Callback */);
		ret = fb->display_ops->update(fb->display_hdl, buff_idx?fb->buff1:fb->buff0, NULL, NULL /* Callback */);
		rhea_clock_stop(fb);
	} else {
		atomic_set(&fb->is_graphics_started, 1);
		if (var->reserved[0] == 0x54445055) {
			region.t	= var->reserved[1] >> 16;
			region.l	= (u16)var->reserved[1]; 
			region.b	= (var->reserved[2] >> 16) -1;
			region.r	= (u16)var->reserved[2] - 1;
			region.w	= region.r - region.l + 1;
			region.h	= region.b - region.t + 1;
			region.mode	= 0;
                        p_region = &region;                
		} else {
			p_region = NULL;	
		}
		wait_for_completion(&fb->prev_buf_done_sem);
		rhea_clock_start(fb);
#ifdef KONAFB_OLD_API
		ret = fb->display_ops->update(fb->display_hdl, buff_idx, 0, (DISPDRV_CB_T)rhea_display_done_cb);
#else
		ret = fb->display_ops->update(fb->display_hdl, buff_idx?fb->buff1:fb->buff0, 0, (DISPDRV_CB_T)rhea_display_done_cb);
#endif
	}
skip_drawing:
	mutex_unlock(&fb->update_sem);

	rheafb_debug("RHEA Display is updated once at %d time with yoffset=%d\n", fb->base_update_count, var->yoffset);
	return ret;
}


#ifdef KONAFB_OLD_API
static void lq043y1dx01_reset(u32 gpio)
{
	gpio_request(gpio, "lcd_reset");
	gpio_direction_output(gpio, 0);
	gpio_set_value_cansleep(gpio, 1);
	msleep(1);
	gpio_set_value_cansleep(gpio, 0);
	msleep(1);
	gpio_set_value_cansleep(gpio, 1);
	msleep(20);
}

static void lq_reset(void)
{
	// Reset SHARP DPI display
	lq043y1dx01_reset(SHARP_RESET);
	
	/* DSI BRIDGE P-ON */
	gpio_request(DSI_BRIDGE_PON, "dsi_bridge_pon");
	gpio_direction_output(DSI_BRIDGE_PON, 0);
	gpio_set_value_cansleep(DSI_BRIDGE_PON, 0);
        msleep(20);
	
	/* Reset DSI Bridge */
	lq043y1dx01_reset(DSI_BRIDGE_RESET);
}

static void reset_display(u32 gpio)
{	
	if (gpio != 0) {
		gpio_request(gpio, "LCD_RST1");
		gpio_direction_output(gpio, 0);
		gpio_set_value_cansleep(gpio, 1);
		msleep(1);
		gpio_set_value_cansleep(gpio, 0);
		msleep(1);
		gpio_set_value_cansleep(gpio, 1);
		msleep(20);
	} else {
		lq_reset();
	}
}
#endif

#ifdef KONAFB_OLD_API
static int enable_display(struct rhea_fb *fb, u32 gpio, u32 bus_width)
{
#else
static int enable_display(struct rhea_fb *fb, struct dispdrv_init_parms* parms)
{
#endif
	int ret = 0;

#ifdef KONAFB_OLD_API
	DISPDRV_OPEN_PARM_T local_DISPDRV_OPEN_PARM_T;
#endif	

#ifdef KONAFB_OLD_API
	ret = fb->display_ops->init(bus_width);
#else
	ret = fb->display_ops->init(parms, &fb->display_hdl);
#endif	
	if (ret != 0) {
		rheafb_error("Failed to init this display device!\n");
		goto fail_to_init;
	}
	
#ifndef KONAFB_OLD_API
	fb->display_info = fb->display_ops->get_info(fb->display_hdl);
#endif
	
#ifdef KONAFB_OLD_API
	reset_display(gpio);
	printk("sleep 100ms after reset gpio %d\n",gpio);
	msleep(20); //haipeng Temporary

	local_DISPDRV_OPEN_PARM_T.busId = fb->phys_fbbase;
	local_DISPDRV_OPEN_PARM_T.busCh = 0;
	ret = fb->display_ops->open((void *)&local_DISPDRV_OPEN_PARM_T, &fb->display_hdl);
#else
	ret = fb->display_ops->start(fb->display_hdl, &fb->dfs_node );
	ret = fb->display_ops->open(fb->display_hdl);
#endif

	if (ret != 0) {
		rheafb_error("Failed to open this display device!\n");
		goto fail_to_open;
	}

	ret = fb->display_ops->power_control(fb->display_hdl, CTRL_PWR_ON);
	if (ret != 0) {
		rheafb_error("Failed to power on this display device!\n");
		goto fail_to_power_control;
 	}
	
#ifndef KONAFB_OLD_API
	ret = fb->display_ops->stop(fb->display_hdl, &fb->dfs_node );
#endif
 	rheafb_info("RHEA display is enabled successfully\n");
	return 0;
 
fail_to_power_control:
	fb->display_ops->close(fb->display_hdl);
fail_to_open:
#ifndef KONAFB_OLD_API
	fb->display_ops->stop(fb->display_hdl, &fb->dfs_node );
#endif
	fb->display_ops->exit(fb->display_hdl);
fail_to_init:
 	return ret;

}

static int disable_display(struct rhea_fb *fb)
{
	int ret = 0;

#ifdef KONAFB_OLD_API
	fb->display_ops->power_control(fb->display_hdl, CTRL_SLEEP_IN);
#else
	fb->display_ops->power_control(fb->display_hdl, CTRL_PWR_OFF);
#endif	
	fb->display_ops->close(fb->display_hdl);

	fb->display_ops->exit(fb->display_hdl);

	rheafb_info("RHEA display is disabled successfully\n");
	return ret;
}

static int rhea_fb_ioctl(struct fb_info *info, unsigned int cmd,
                        unsigned long arg)
{   
	void *ptr = NULL;
	int ret = 0;
	struct rhea_fb *fb = container_of(info, struct rhea_fb, fb);

	rheafb_debug("RHEA ioctl called! Cmd %x, Arg %lx\n", cmd, arg);
	switch (cmd) {
        
  	case RHEA_IOCTL_SET_BUFFER_AND_UPDATE:
        
		if (mutex_lock_killable(&fb->update_sem)) {
		    return -EINTR;
		}
		ptr = (void *)arg;

		if (ptr == NULL)
		{
		    mutex_unlock(&fb->update_sem);
		    return -EFAULT;
		}

		wait_for_completion(&fb->prev_buf_done_sem);
		rhea_clock_start(fb);
//		ret = fb->display_ops->update(fb->display_hdl, (uint32_t)ptr, (DISPDRV_WIN_t *)1, NULL/*(DISPDRV_CB_T)rhea_display_done_cb*/);
		ret = fb->display_ops->update(fb->display_hdl, ptr, NULL, NULL/*(DISPDRV_CB_T)rhea_display_done_cb*/);
		rhea_clock_stop(fb);
		complete(&g_rhea_fb->prev_buf_done_sem);
		mutex_unlock(&fb->update_sem);
		break;

	default:
	
		rheafb_error("Wrong ioctl cmd\n");
		break;
	}

	return ret;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void rhea_fb_early_suspend(struct early_suspend *h)
{
	struct rhea_fb *fb;

	rheafb_error("BRCM fb early suspend with level = %d\n", h->level);

	switch (h->level) {
	
	case EARLY_SUSPEND_LEVEL_BLANK_SCREEN:
		/* Turn off the backlight */
		fb = container_of(h, struct rhea_fb, early_suspend_level1);
		mutex_lock(&fb->update_sem);
		wait_for_completion(&fb->prev_buf_done_sem);
	 	rhea_clock_start(fb);
		if (fb->display_ops->power_control(fb->display_hdl, CTRL_SCREEN_OFF))
			rheafb_error("Failed to blank this display device!\n");
		rhea_clock_stop(fb);
		complete(&g_rhea_fb->prev_buf_done_sem);
		mutex_unlock(&fb->update_sem);

		break;

	case EARLY_SUSPEND_LEVEL_STOP_DRAWING:
		fb = container_of(h, struct rhea_fb, early_suspend_level2);
		mutex_lock(&fb->update_sem);
		wait_for_completion(&fb->prev_buf_done_sem);
		fb->g_stop_drawing = 1;
		complete(&g_rhea_fb->prev_buf_done_sem);
		mutex_unlock(&fb->update_sem);
		break;

	case EARLY_SUSPEND_LEVEL_DISABLE_FB:
		fb = container_of(h, struct rhea_fb, early_suspend_level3);
		/* screen goes to sleep mode*/
		mutex_lock(&fb->update_sem);
	 	rhea_clock_start(fb);
		disable_display(fb);
		rhea_clock_stop(fb);
		mutex_unlock(&fb->update_sem);
		/* Ok for MM going to shutdown state */
		pi_mgr_qos_request_update(&g_mm_qos_node, PI_MGR_QOS_DEFAULT_VALUE);
		break;

	default:
		rheafb_error("Early suspend with the wrong level!\n");
		break;
	}
}

static void rhea_fb_late_resume(struct early_suspend *h)
{
	struct rhea_fb *fb;

	rheafb_error("BRCM fb late resume with level = %d\n", h->level);

	switch (h->level) {
	
	case EARLY_SUSPEND_LEVEL_BLANK_SCREEN:
		/* Turn on the backlight */
		fb = container_of(h, struct rhea_fb, early_suspend_level1);
#ifndef KONAFB_OLD_API
	 	rhea_clock_start(fb);
		if (fb->display_ops->power_control(fb->display_hdl, CTRL_SCREEN_ON))
			rheafb_error("Failed to unblank this display device!\n");
		rhea_clock_stop(fb);
#endif
		break;

	case EARLY_SUSPEND_LEVEL_STOP_DRAWING:
		fb = container_of(h, struct rhea_fb, early_suspend_level2);
		mutex_lock(&fb->update_sem);
		fb->g_stop_drawing = 0;
		mutex_unlock(&fb->update_sem);
		break;

	case EARLY_SUSPEND_LEVEL_DISABLE_FB:
		fb = container_of(h, struct rhea_fb, early_suspend_level3);
		/* Ok for MM going to retention but not shutdown state */
		pi_mgr_qos_request_update(&g_mm_qos_node, 10);
		/* screen comes out of sleep */
#ifdef KONAFB_OLD_API
	 	rhea_clock_start(fb);
		if (enable_display(fb, fb->gpio, fb->bus_width))
			rheafb_error("Failed to enable this display device\n");
		rhea_clock_stop(fb);
#else
		if (enable_display(fb, &fb->lcd_drv_parms))
			rheafb_error("Failed to enable this display device\n");
#endif
		break;

	default:
		rheafb_error("Early suspend with the wrong level!\n");
		break;
	}


}
#endif

#if !defined(CONFIG_MACH_RHEA_RAY_EDN1X) && !defined(CONFIG_MACH_RHEA_BERRI) && !defined(CONFIG_MACH_RHEA_RAY_EDN2X) && !defined(CONFIG_MACH_RHEA_SS_COMMON) \
	&& !defined(CONFIG_MACH_RHEA_RAY_DEMO) && !defined(CONFIG_MACH_RHEA_BERRI_EDN40)  && !defined(CONFIG_MACH_RHEA_STONE)
static int rhea_refresh_thread(void *arg)
{
	struct rhea_fb *fb = arg;

	wait_for_completion(&fb->thread_sem);

	do {
		wait_for_completion(&fb->refresh_wait_sem);
		mutex_lock(&fb->update_sem);
		if (0 == fb->g_stop_drawing) {
			rhea_clock_start(fb);
#ifdef KONAFB_OLD_API
			fb->display_ops->update(fb->display_hdl, 0, NULL, NULL);
#else
			fb->display_ops->update(fb->display_hdl, fb->buff0, NULL, NULL);
#endif			
			rhea_clock_stop(fb);
			fb->base_update_count++;
		}
		mutex_unlock(&fb->update_sem);
	} while (1);

	rheafb_debug("RHEA refresh thread is exiting!\n");
	return 0;
}

static int vt_notifier_call(struct notifier_block *blk,
			    unsigned long code, void *_param)
{	
	switch (code) {
	case VT_UPDATE:
		complete(&g_rhea_fb->refresh_wait_sem);
		break;
	}

	return 0;
}

static struct notifier_block vt_notifier_block = {
	.notifier_call = vt_notifier_call,
};

#endif /* !CONFIG_MACH_RHEA_RAY_EDN1X  && !CONFIG_MACH_RHEA_RAY_EDN2X && !CONFIG_MACH_RHEA_SS_COMMON */

static struct fb_ops rhea_fb_ops = {
	.owner          = THIS_MODULE,
	.fb_check_var   = rhea_fb_check_var,
	.fb_set_par     = rhea_fb_set_par,
	.fb_setcolreg   = rhea_fb_setcolreg,
	.fb_pan_display = rhea_fb_pan_display,
	.fb_fillrect    = cfb_fillrect,
	.fb_copyarea    = cfb_copyarea,
	.fb_imageblit   = cfb_imageblit,
	.fb_ioctl       = rhea_fb_ioctl,
};

static int rhea_fb_probe(struct platform_device *pdev)
{
	int ret = -ENXIO;
	struct rhea_fb *fb;
	size_t framesize;
	uint32_t width, height;
	int ret_val = -1;

	struct kona_fb_platform_data *fb_data;
#ifndef KONAFB_OLD_API
	printk("%s:+\n",__func__);
#endif
	if (g_rhea_fb && (g_rhea_fb->is_display_found == 1)) {
		rheafb_info("A right display device is already found!\n");
		return -EINVAL;
	}

	fb = kzalloc(sizeof(struct rhea_fb), GFP_KERNEL);
	if (fb == NULL) {
		rheafb_error("Unable to allocate framebuffer structure\n");
		ret = -ENOMEM;
		goto err_fb_alloc_failed;
	}
	fb->g_stop_drawing = 0;

	g_rhea_fb = fb;
 	ret_val = pi_mgr_dfs_add_request(&g_rhea_fb->dfs_node,"lcd", PI_MGR_PI_ID_MM, PI_MGR_DFS_MIN_VALUE);
	if (ret_val)
	{
		printk(KERN_ERR "Failed to add dfs request for LCD\n");
		ret = -EIO;
		goto fb_data_failed;
	}

	fb_data = pdev->dev.platform_data;
	if (!fb_data) {
		ret = -EINVAL;
		goto fb_data_failed;
	}
#ifdef KONAFB_OLD_API
	fb->display_ops = 
		(DISPDRV_T *)fb_data->get_dispdrv_func_tbl();
#else
	fb->display_ops = 
		(DISPDRV_T *)fb_data->dispdrv_entry();
#endif

	spin_lock_init(&fb->lock);
	platform_set_drvdata(pdev, fb);

	mutex_init(&fb->update_sem);
	atomic_set(&fb->buff_idx, 0);
	atomic_set(&fb->is_fb_registered, 0);
	init_completion(&fb->prev_buf_done_sem);
	complete(&fb->prev_buf_done_sem); // First attmept should pass
	atomic_set(&fb->is_graphics_started, 0);
	init_completion(&fb->thread_sem);

//+GG PROBE DISPLAY
#ifndef KONAFB_OLD_API
	/* enable_display will start/stop clocks on its own */
	fb->lcd_drv_parms = *(struct dispdrv_init_parms *)&fb_data->parms;
	ret = enable_display(fb, &fb->lcd_drv_parms);
	
	if (ret) {
		rheafb_error("Failed to enable this display device\n");
		goto err_enable_display_failed;
	} else {
		fb->is_display_found = 1;
 	}
	
#if (RHEA_FB_ENABLE_DYNAMIC_CLOCK != 1)
	//GG Start the clock here if not DYNAMIC
	fb->display_ops->start(&fb->dfs_node);
#endif
#endif // #ifndef KONAFB_OLD_API
//-GG PROBE DISPLAY

#if !defined(CONFIG_MACH_RHEA_RAY_EDN1X) && !defined(CONFIG_MACH_RHEA_BERRI) && !defined(CONFIG_MACH_RHEA_RAY_EDN2X) && !defined(CONFIG_MACH_RHEA_SS_COMMON) \
	&& !defined(CONFIG_MACH_RHEA_RAY_DEMO) && !defined(CONFIG_MACH_RHEA_BERRI_EDN40) && !defined(CONFIG_MACH_RHEA_STONE)

	init_completion(&fb->refresh_wait_sem);

	fb->thread = kthread_run(rhea_refresh_thread, fb, "lcdrefresh_d");
	if (IS_ERR(fb->thread)) {
		ret = PTR_ERR(fb->thread);
		goto thread_create_failed;
	}
#endif

#ifdef KONAFB_OLD_API
	framesize = fb_data->screen_width * fb_data->screen_height * 
				fb_data->bytes_per_pixel * 2;
#else
	framesize = fb->display_info->width * fb->display_info->height * 
				fb->display_info->Bpp * 2;
#endif

	fb->fb.screen_base = dma_alloc_writecombine(&pdev->dev,
			framesize, &fb->phys_fbbase, GFP_KERNEL);
	if (fb->fb.screen_base == NULL) {
		ret = -ENOMEM;
		rheafb_error("Unable to allocate fb memory\n");
		goto err_fbmem_alloc_failed;
	}

// old way pf probing display
#ifdef KONAFB_OLD_API
#if (RHEA_FB_ENABLE_DYNAMIC_CLOCK != 1)
	//GG Start the clock here if not DYNAMIC
	fb->display_ops->start(&fb->dfs_node);
#endif

	fb->gpio = fb_data->gpio;
	fb->bus_width = fb_data->bus_width;
	rhea_clock_start(fb);
	ret = enable_display(fb, fb->gpio, fb->bus_width);
	if (ret) {
		rheafb_error("Failed to enable this display device\n");
		goto err_enable_display_failed;
	} else {
		fb->is_display_found = 1;
 	}
	rhea_clock_stop(fb);

	fb->display_info = fb->display_ops->get_info(fb->display_hdl);
#endif

	/* Now we should get correct width and height for this display .. */
	width = fb->display_info->width; 
	height = fb->display_info->height;
#ifdef KONAFB_OLD_API
	BUG_ON(width != fb_data->screen_width || height != fb_data->screen_height);
#endif

#ifndef KONAFB_OLD_API
	fb->buff0 = (void*)fb->phys_fbbase;
	fb->buff1 = (void*)fb->phys_fbbase +  width * height * fb->display_info->Bpp;
#endif

	fb->fb.fbops		= &rhea_fb_ops;
	fb->fb.flags		= FBINFO_FLAG_DEFAULT;
	fb->fb.pseudo_palette	= fb->cmap;
	fb->fb.fix.type		= FB_TYPE_PACKED_PIXELS;
	fb->fb.fix.visual	= FB_VISUAL_TRUECOLOR;
#ifdef KONAFB_OLD_API
	fb->fb.fix.line_length	= width * fb_data->bytes_per_pixel;
#else
	fb->fb.fix.line_length	= width * fb->display_info->Bpp;
#endif
	fb->fb.fix.accel	= FB_ACCEL_NONE;
	fb->fb.fix.ypanstep	= 1;
	fb->fb.fix.xpanstep	= 4;
#ifdef PARTIAL_UPDATE_SUPPORT
	fb->fb.fix.reserved[0]	=  0x5444;
	fb->fb.fix.reserved[1]	=  0x5055;
#endif

	fb->fb.var.xres		= width;
	fb->fb.var.yres		= height;
	fb->fb.var.xres_virtual	= width;
	fb->fb.var.yres_virtual	= height * 2;
#ifdef KONAFB_OLD_API
	fb->fb.var.bits_per_pixel = fb_data->bytes_per_pixel * 8;
#else
	fb->fb.var.bits_per_pixel = fb->display_info->Bpp << 3;
#endif	
	fb->fb.var.activate	= FB_ACTIVATE_NOW;
	fb->fb.var.height	= fb->display_info->phys_height;
	fb->fb.var.width	= fb->display_info->phys_width;

#ifdef CONFIG_CDEBUGGER
	/* it has dependency on h/w */
	frame_buf_mark.p_fb = (void *)(fb->phys_fbbase - PHYS_OFFSET);
	frame_buf_mark.resX = fb->fb.var.xres;
	frame_buf_mark.resY = fb->fb.var.yres;
	frame_buf_mark.bpp = fb->fb.var.bits_per_pixel;
#endif

#ifdef KONAFB_OLD_API
	switch (fb_data->pixel_format) {
#else
	switch (fb->display_info->input_format) {
#endif

#ifdef KONAFB_OLD_API
	case RGB565:
#else
	case DISPDRV_FB_FORMAT_RGB565:
#endif	
	fb->fb.var.red.offset = 11;
	fb->fb.var.red.length = 5;
	fb->fb.var.green.offset = 5;
	fb->fb.var.green.length = 6;
	fb->fb.var.blue.offset = 0;
	fb->fb.var.blue.length = 5;

	framesize = width * height * 2 * 2;
	break;

#ifdef KONAFB_OLD_API
	case XRGB8888:
#else
	case DISPDRV_FB_FORMAT_RGB888_U:
#endif	
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
	rheafb_error("Wrong format!\n");
	break;
	}

	fb->fb.fix.smem_start = fb->phys_fbbase;
	fb->fb.fix.smem_len = framesize;

	rheafb_debug("Framebuffer starts at phys[0x%08x], and virt[0x%08x] with frame size[0x%08x]\n",
			fb->phys_fbbase, (uint32_t)fb->fb.screen_base, framesize);

	ret = fb_set_var(&fb->fb, &fb->fb.var);
	if (ret) {
		rheafb_error("fb_set_var failed\n");
		goto err_set_var_failed;
	}
	/* Paint it black (assuming default fb contents are all zero) */
	ret = rhea_fb_pan_display(&fb->fb.var, &fb->fb);
	if (ret) {
		rheafb_error("Can not enable the LCD!\n");
		goto err_fb_register_failed;
	}
	
	if (fb->display_ops->set_brightness) {
		rhea_clock_start(fb);
		fb->display_ops->set_brightness(fb->display_hdl,100);
		rhea_clock_stop(fb);
	}

#ifndef KONAFB_OLD_API
	/* display on after painted blank */
	rhea_clock_start(fb);
	fb->display_ops->power_control(fb->display_hdl, CTRL_SCREEN_ON);
	rhea_clock_stop(fb);
#endif

	ret = register_framebuffer(&fb->fb);
	if (ret) {
		rheafb_error("Framebuffer registration failed\n");
		goto err_fb_register_failed;
	}

#ifdef CONFIG_FRAMEBUFFER_FPS
	fb->fps_info = fb_fps_register(&fb->fb);	
	if (NULL == fb->fps_info )
		printk(KERN_ERR "No fps display");
#endif
	complete(&fb->thread_sem);

	atomic_set(&fb->is_fb_registered, 1);
	rheafb_info("RHEA Framebuffer probe successfull\n");

#if !defined(CONFIG_MACH_RHEA_RAY_EDN1X) && !defined(CONFIG_MACH_RHEA_BERRI) && !defined(CONFIG_MACH_RHEA_RAY_EDN2X) && !defined(CONFIG_MACH_RHEA_SS_COMMON) \
	&& !defined(CONFIG_MACH_RHEA_RAY_DEMO) && !defined(CONFIG_MACH_RHEA_BERRI_EDN40) && !defined(CONFIG_MACH_RHEA_STONE)
	register_vt_notifier(&vt_notifier_block);
#endif

#ifdef CONFIG_LOGO
	fb_prepare_logo(&fb->fb, 0);
	fb_show_logo(&fb->fb, 0);

	mutex_lock(&fb->update_sem);
	rhea_clock_start(fb);

#ifdef KONAFB_OLD_API
	fb->display_ops->update(fb->display_hdl, 0, NULL, NULL);
#else
	fb->display_ops->update(fb->display_hdl, fb->buff0, NULL, NULL);
#endif			
	rhea_clock_stop(fb);
	mutex_unlock(&fb->update_sem);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	fb->early_suspend_level1.suspend	= rhea_fb_early_suspend;
	fb->early_suspend_level1.resume	= rhea_fb_late_resume;
	fb->early_suspend_level1.level		=  EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	register_early_suspend(&fb->early_suspend_level1);

	fb->early_suspend_level2.suspend	= rhea_fb_early_suspend;
	fb->early_suspend_level2.resume	= rhea_fb_late_resume;
	fb->early_suspend_level2.level		=  EARLY_SUSPEND_LEVEL_STOP_DRAWING;
	register_early_suspend(&fb->early_suspend_level2);

	fb->early_suspend_level3.suspend	= rhea_fb_early_suspend;
	fb->early_suspend_level3.resume	= rhea_fb_late_resume;
	fb->early_suspend_level3.level		=  EARLY_SUSPEND_LEVEL_DISABLE_FB;
	register_early_suspend(&fb->early_suspend_level3);
#endif

	return 0;

err_fb_register_failed:
err_set_var_failed:
	dma_free_writecombine(&pdev->dev, fb->fb.fix.smem_len,
			      fb->fb.screen_base, fb->fb.fix.smem_start);

	rhea_clock_start(fb);
	disable_display(fb);
	rhea_clock_stop(fb);

#if (RHEA_FB_ENABLE_DYNAMIC_CLOCK != 1)
	fb->display_ops->stop(&fb->dfs_node);
#endif

err_enable_display_failed:
err_fbmem_alloc_failed:
#if !defined(CONFIG_MACH_RHEA_RAY_EDN1X) && !defined(CONFIG_MACH_RHEA_BERRI) && !defined(CONFIG_MACH_RHEA_RAY_EDN2X) && !defined(CONFIG_MACH_RHEA_SS_COMMON) \
	&& !defined(CONFIG_MACH_RHEA_RAY_DEMO) && !defined(CONFIG_MACH_RHEA_BERRI_EDN40) && !defined(CONFIG_MACH_RHEA_STONE)
thread_create_failed:
#endif
	if (pi_mgr_dfs_request_remove(&fb->dfs_node))
	{
	    printk(KERN_ERR "Failed to remove dfs request for LCD\n");
	}
fb_data_failed:
	kfree(fb);
	g_rhea_fb = NULL;
err_fb_alloc_failed:
	return ret;
}

static int __devexit rhea_fb_remove(struct platform_device *pdev)
{
	size_t framesize;
	struct rhea_fb *fb = platform_get_drvdata(pdev);
	
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
	rheafb_info("RHEA FB removed !!\n");
	return 0;
}

static struct platform_driver rhea_fb_driver = {
	.probe		= rhea_fb_probe,
	.remove		= __devexit_p(rhea_fb_remove),
	.driver = {
		.name = "rhea_fb"
	}
};

static int __init rhea_fb_init(void)
{
	int ret;

	ret = pi_mgr_qos_add_request(&g_mm_qos_node, "lcd", PI_MGR_PI_ID_MM, 10);
	if (ret)
		printk(KERN_ERR "failed to register qos client for lcd\n");

	ret = platform_driver_register(&rhea_fb_driver);
	if (ret) {
		printk(KERN_ERR"%s : Unable to register Rhea framebuffer driver\n", __func__);
		goto fail_to_register;
	}
fail_to_register:
	printk(KERN_INFO"BRCM Framebuffer Init %s !\n", ret ? "FAILED" : "OK");

	return ret;
}

static void __exit rhea_fb_exit(void)
{
	platform_driver_unregister(&rhea_fb_driver);
	printk(KERN_INFO"BRCM Framebuffer exit OK\n");
}

late_initcall(rhea_fb_init);
module_exit(rhea_fb_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("RHEA FB Driver");
