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
#include <video/kona_fb.h>

#include <mach/io.h>

#ifdef CONFIG_FRAMEBUFFER_FPS
#include <linux/fb_fps.h>
#endif

#ifdef CONFIG_ANDROID_POWER
#include <linux/android_power.h>
#endif

//#define RHEA_FB_DEBUG 
#include "rhea_fb.h"
#include <plat/mobcom_types.h>
#include "lcd/display_drv.h"

//extern DISPDRV_T* DISP_DRV_NT35582_WVGA_SMI_GetFuncTable ( void );
//extern DISPDRV_T* DISP_DRV_BCM91008_ALEX_GetFuncTable( void );
//extern DISPDRV_T* DISP_DRV_R61581_HVGA_SMI_GetFuncTable ( void );

struct rhea_fb {
	dma_addr_t phys_fbbase;
	spinlock_t lock;
	struct task_struct *thread;
	struct semaphore thread_sem;
	struct semaphore update_sem;
	struct semaphore prev_buf_done_sem;
#ifndef CONFIG_MACH_RHEA_RAY_EDN1X
	struct semaphore refresh_wait_sem;
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
#ifdef CONFIG_ANDROID_POWER
	android_early_suspend_t early_suspend;
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

	if((var->xoffset != info->var.xoffset) ||
	   (var->bits_per_pixel != info->var.bits_per_pixel) ||
	   (var->grayscale != info->var.grayscale)) {
		rheafb_error("fb_check_var_failed\n");
		return -EINVAL;
	}

	if ((var->yoffset != 0) &&
		(var->yoffset != info->var.yres)) {
		rheafb_error("fb_check_var failed\n");
		rheafb_alert("BRCM fb does not support partial FB updates\n");
		return -EINVAL;
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

static void rhea_display_done_cb(int status)
{	
	(void)status;
	up(&g_rhea_fb->prev_buf_done_sem);
}

static int rhea_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	int ret = 0;
	struct rhea_fb *fb = container_of(info, struct rhea_fb, fb);
	uint32_t buff_idx;
#ifdef CONFIG_FRAMEBUFFER_FPS
	void *dst;
#endif

	/* We are here only if yoffset is '0' or 'yres',
	 * so if yoffset = 0, update first buffer or update second
	 */
	buff_idx = var->yoffset ? 1 : 0;

	rheafb_debug("RHEA %s with buff_idx =%d \n", __func__, buff_idx);

	if (down_killable(&fb->update_sem))
		return -EINTR;

	atomic_set(&fb->buff_idx, buff_idx);

#ifdef CONFIG_FRAMEBUFFER_FPS
	dst = (fb->fb.screen_base) + 
		(buff_idx * fb->fb.var.xres * fb->fb.var.yres * (fb->fb.var.bits_per_pixel/8));
	fb_fps_display(fb->fps_info, dst, 5, 2, 0);
#endif
	
	if (!atomic_read(&fb->is_fb_registered)) {
		ret = fb->display_ops->update(fb->display_hdl, buff_idx, NULL /* Callback */);
	} else {
		atomic_set(&fb->is_graphics_started, 1);
		down(&fb->prev_buf_done_sem);
		ret = fb->display_ops->update(fb->display_hdl, buff_idx,(DISPDRV_CB_T)rhea_display_done_cb);
	}
	
	up(&fb->update_sem);

	rheafb_debug("RHEA Display is updated once at %d time with yoffset=%d\n", fb->base_update_count, var->yoffset);

	return ret;
}

#ifdef CONFIG_ANDROID_POWER
static void rhea_fb_early_suspend(android_early_suspend_t *h)
{
	struct rhea_fb *fb = container_of(h, struct rhea_fb, early_suspend);
	rheafb_info("TODO: BRCM fb early suspend ...\n");
}

static void rhea_fb_late_resume(android_early_suspend_t *h)
{
	struct rhea_fb *fb = container_of(h, struct rhea_fb, early_suspend);
	rheafb_info("TODO: BRCM fb late resume ...\n");
}
#endif

static int enable_display(struct rhea_fb *fb)
{
	int ret = 0;
	DISPDRV_OPEN_PARM_T local_DISPDRV_OPEN_PARM_T;

	ret = fb->display_ops->init();
	if (ret != 0) {
		rheafb_error("Failed to init this display device!\n");
		goto fail_to_init;
	}
	
	/* Hack
	 * Since the display driver is not using this field, 
	 * we use it to pass the dma addr.
	 */
	local_DISPDRV_OPEN_PARM_T.busId = fb->phys_fbbase;
	local_DISPDRV_OPEN_PARM_T.busCh = 0;
	ret = fb->display_ops->open((void *)&local_DISPDRV_OPEN_PARM_T, &fb->display_hdl);
	if (ret != 0) {
		rheafb_error("Failed to open this display device!\n");
		goto fail_to_open;
	}

	ret = fb->display_ops->start(fb->display_hdl);
	if (ret != 0) {
		rheafb_error("Failed to start this display device!\n");
		goto fail_to_start;
	}

	ret = fb->display_ops->power_control(fb->display_hdl, DISPLAY_POWER_STATE_ON);
	if (ret != 0) {
		rheafb_error("Failed to power on this display device!\n");
		goto fail_to_power_control;
 	}

 	rheafb_info("RHEA display is enabled successfully\n");
	return 0;
 
fail_to_power_control:
	fb->display_ops->stop(fb->display_hdl);
fail_to_start:
	fb->display_ops->close(fb->display_hdl);
fail_to_open:
	fb->display_ops->exit();
fail_to_init:
 	return ret;

}
#if 0

#if defined(CONFIG_FB_BRCM_LCDC_ALEX_DSI_VGA)
	fb->display_ops = DISP_DRV_BCM91008_ALEX_GetFuncTable();
#elif defined(CONFIG_FB_BRCM_LCDC_NT35582_SMI_WVGA)
	fb->display_ops = DISP_DRV_NT35582_WVGA_SMI_GetFuncTable();
#elif defined(CONFIG_FB_BRCM_LCDC_R61581_SMI_HVGA)
	fb->display_ops = DISP_DRV_R61581_HVGA_SMI_GetFuncTable(); 
#else 
#error "Wrong LCD configuration!" 
#endif
	fb->display_ops->init();
	{
		DISPDRV_OPEN_PARM_T local_DISPDRV_OPEN_PARM_T;
		/* Hack
		 * Since the smi display driver is not using this field, 
		 * we use it to pass the dma addr.
		 */
		local_DISPDRV_OPEN_PARM_T.busId = fb->phys_fbbase;
		local_DISPDRV_OPEN_PARM_T.busCh = 0;
		fb->display_ops->open((void *)&local_DISPDRV_OPEN_PARM_T, &fb->display_hdl);
		fb->display_ops->start(fb->display_hdl);
		fb->display_ops->power_control(fb->display_hdl, DISPLAY_POWER_STATE_ON);
	}
	rheafb_info("RHEA display is enabled successfully\n");

	return ret;
}

#endif

static int disable_display(struct rhea_fb *fb)
{
	int ret = 0;

	/* TODO:  HACK
	 * Need to fill the blank.
	 */
	rheafb_info("RHEA display is disabled successfully\n");
	return ret;
}

#ifndef CONFIG_MACH_RHEA_RAY_EDN1X
static int rhea_refresh_thread(void *arg)
{
	struct rhea_fb *fb = arg;

	down(&fb->thread_sem);

	do {
		down(&fb->refresh_wait_sem);
		down(&fb->update_sem);
		fb->display_ops->update(fb->display_hdl, 0, NULL);
		fb->base_update_count++;
		up(&fb->update_sem);
	} while (1);

	rheafb_debug("RHEA refresh thread is exiting!\n");
	return 0;
}

static int vt_notifier_call(struct notifier_block *blk,
			    unsigned long code, void *_param)
{	
	switch (code) {
	case VT_UPDATE:
		up(&g_rhea_fb->refresh_wait_sem);
		break;
	}

	return 0;
}

static struct notifier_block vt_notifier_block = {
	.notifier_call = vt_notifier_call,
};

#endif /* !CONFIG_MACH_RHEA_RAY_EDN1X */

static struct fb_ops rhea_fb_ops = {
	.owner          = THIS_MODULE,
	.fb_check_var   = rhea_fb_check_var,
	.fb_set_par     = rhea_fb_set_par,
	.fb_setcolreg   = rhea_fb_setcolreg,
	.fb_pan_display = rhea_fb_pan_display,
	.fb_fillrect    = cfb_fillrect,
	.fb_copyarea    = cfb_copyarea,
	.fb_imageblit   = cfb_imageblit,
};

static int rhea_fb_probe(struct platform_device *pdev)
{
	int ret = -ENXIO;
	struct rhea_fb *fb;
	size_t framesize;
	uint32_t width, height;

	struct kona_fb_platform_data *fb_data;

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
	g_rhea_fb = fb;

	fb_data = pdev->dev.platform_data;
	if (!fb_data) {
		ret = -EINVAL;
		goto fb_data_failed;
	}
	fb->display_ops = 
		(DISPDRV_T *)fb_data->get_dispdrv_func_tbl();

	spin_lock_init(&fb->lock);
	platform_set_drvdata(pdev, fb);

	sema_init(&fb->update_sem, 1);
	atomic_set(&fb->buff_idx, 0);
	atomic_set(&fb->is_fb_registered, 0);
	sema_init(&fb->prev_buf_done_sem, 1);
	atomic_set(&fb->is_graphics_started, 0);
	sema_init(&fb->thread_sem, 0);

#ifndef CONFIG_MACH_RHEA_RAY_EDN1X
	sema_init(&fb->refresh_wait_sem, 0);

	fb->thread = kthread_run(rhea_refresh_thread, fb, "lcdrefresh_d");
	if (IS_ERR(fb->thread)) {
		ret = PTR_ERR(fb->thread);
		goto thread_create_failed;
	}
#endif

	/* Hack
	 * The screen info can only be obtained from the display driver;and, therefore, 
	 * only then the frame buffer mem can be allocated.
	 * However, we need to pass the frame buffer phy addr into the CSL layer right before
	 * the screen info can be obtained.
	 * So either the display driver allocates the memory and pass the pointer to us, or
	 * we allocate memory and pass into the display. 
	 */
#if 0
#if defined(CONFIG_FB_BRCM_LCDC_ALEX_DSI_VGA)
	framesize = 640 * 360 * 4 * 2;
#elif defined(CONFIG_FB_BRCM_LCDC_NT35582_SMI_WVGA)
	framesize = 480 * 800 * 2 * 2;
#elif defined(CONFIG_FB_BRCM_LCDC_R61581_SMI_HVGA)
	framesize = 320 * 480 * 2 * 2;
#else 
#error "Wrong LCD configuration!" 
#endif
#endif
	framesize = fb_data->screen_width * fb_data->screen_height * 
				fb_data->bytes_per_pixel * 2;

	fb->fb.screen_base = dma_alloc_writecombine(&pdev->dev,
			framesize, &fb->phys_fbbase, GFP_KERNEL);
	if (fb->fb.screen_base == NULL) {
		ret = -ENOMEM;
		rheafb_error("Unable to allocate fb memory\n");
		goto err_fbmem_alloc_failed;
	}

	ret = enable_display(fb);
	if (ret) {
		rheafb_error("Failed to enable this display device\n");
		goto err_enable_display_failed;
	} else {
		fb->is_display_found = 1;
 	}

	fb->display_info = fb->display_ops->get_info(fb->display_hdl);

	/* Now we should get correct width and height for this display .. */
	width = fb->display_info->width; 
	height = fb->display_info->height;
	BUG_ON(width != fb_data->screen_width || height != fb_data->screen_height);

	fb->fb.fbops		= &rhea_fb_ops;
	fb->fb.flags		= FBINFO_FLAG_DEFAULT;
	fb->fb.pseudo_palette	= fb->cmap;
	fb->fb.fix.type		= FB_TYPE_PACKED_PIXELS;
	fb->fb.fix.visual	= FB_VISUAL_TRUECOLOR;
#if 0
#ifdef CONFIG_FB_BRCM_LCDC_ALEX_DSI_VGA
	fb->fb.fix.line_length	= width * 4;
#else
	fb->fb.fix.line_length	= width * 2;
#endif
#endif
	fb->fb.fix.line_length	= width * fb_data->bytes_per_pixel;

	fb->fb.fix.accel	= FB_ACCEL_NONE;
	fb->fb.fix.ypanstep	= 1;
	fb->fb.fix.xpanstep	= 4;

	fb->fb.var.xres		= width;
	fb->fb.var.yres		= height;
	fb->fb.var.xres_virtual	= width;
	fb->fb.var.yres_virtual	= height * 2;
#if 0
#ifdef CONFIG_FB_BRCM_LCDC_ALEX_DSI_VGA
	fb->fb.var.bits_per_pixel = 32;
#else
	fb->fb.var.bits_per_pixel = 16;
#endif
#endif
	fb->fb.var.bits_per_pixel = fb_data->bytes_per_pixel * 8;
	fb->fb.var.activate	= FB_ACTIVATE_NOW;
	fb->fb.var.height	= height;
	fb->fb.var.width	= width;

#if 0
#ifdef CONFIG_FB_BRCM_LCDC_ALEX_DSI_VGA
	fb->fb.var.red.offset = 16;
	fb->fb.var.red.length = 8;
	fb->fb.var.green.offset = 8;
	fb->fb.var.green.length = 8;
	fb->fb.var.blue.offset = 0;
	fb->fb.var.blue.length = 8;
	fb->fb.var.transp.offset = 24;
	fb->fb.var.transp.length = 8;

	framesize = width * height * 4 * 2;
#else
	fb->fb.var.red.offset = 11;
	fb->fb.var.red.length = 5;
	fb->fb.var.green.offset = 5;
	fb->fb.var.green.length = 6;
	fb->fb.var.blue.offset = 0;
	fb->fb.var.blue.length = 5;

	framesize = width * height * 2 * 2;
#endif
#endif
	switch (fb_data->pixel_format) {
	case RGB565:
	fb->fb.var.red.offset = 11;
	fb->fb.var.red.length = 5;
	fb->fb.var.green.offset = 5;
	fb->fb.var.green.length = 6;
	fb->fb.var.blue.offset = 0;
	fb->fb.var.blue.length = 5;

	framesize = width * height * 2 * 2;
	break;

	case XRGB8888:
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
		goto err_enable_display_failed;
	}

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
	up(&fb->thread_sem);

	atomic_set(&fb->is_fb_registered, 1);
	rheafb_info("RHEA Framebuffer probe successfull\n");

#ifndef CONFIG_MACH_RHEA_RAY_EDN1X
	register_vt_notifier(&vt_notifier_block);
#endif

#ifdef CONFIG_LOGO
	/*  Display the default logo/splash screen. */
	fb_prepare_logo(&fb->fb, 0);
	fb_show_logo(&fb->fb, 0);
	fb->display_ops->update(fb->display_hdl, 0, NULL /* Callback */);
#endif


#ifdef CONFIG_ANDROID_POWER
	fb->early_suspend.suspend = rhea_fb_early_suspend;
	fb->early_suspend.resume = rhea_fb_late_resume;
	android_register_early_suspend(&fb->early_suspend);
#endif

	return 0;

err_fb_register_failed:
err_set_var_failed:
	dma_free_writecombine(&pdev->dev, fb->fb.fix.smem_len,
			      fb->fb.screen_base, fb->fb.fix.smem_start);
	disable_display(fb);

err_enable_display_failed:
err_fbmem_alloc_failed:
#ifndef CONFIG_MACH_RHEA_RAY_EDN1X
thread_create_failed:
#endif
fb_data_failed:
	kfree(fb);
	g_rhea_fb = NULL;
err_fb_alloc_failed:
	rheafb_alert("RHEA Framebuffer probe FAILED !!\n");
	return ret;
}

static int __devexit rhea_fb_remove(struct platform_device *pdev)
{
	size_t framesize;
	struct rhea_fb *fb = platform_get_drvdata(pdev);
	
	framesize = fb->fb.var.xres_virtual * fb->fb.var.yres_virtual * 2;

#ifdef CONFIG_ANDROID_POWER
        android_unregister_early_suspend(&fb->early_suspend);
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

#if 0
static struct platform_device rhea_fb_device = {
	.name    = "rhea_fb",
	.id      = -1,
	.dev = {
		.dma_mask      = (u64 *) ~(u32)0,
		.coherent_dma_mask   = ~(u32)0,
	},
};
#endif

static int __init rhea_fb_init(void)
{
	int ret;

	ret = platform_driver_register(&rhea_fb_driver);
	if (ret) {
		printk(KERN_ERR"%s : Unable to register Rhea framebuffer driver\n", __func__);
		goto fail_to_register;
	}

#if 0
	ret = platform_device_register(&rhea_fb_device);
	if (ret) {
		printk(KERN_ERR"%s : Unable to register Rhea framebuffer device\n", __func__);
		platform_driver_unregister(&rhea_fb_driver);
		goto fail_to_register;
	}
#endif

fail_to_register:
	printk(KERN_INFO"BRCM Framebuffer Init %s !\n", ret ? "FAILED" : "OK");

	return ret;
}

static void __exit rhea_fb_exit(void)
{
	/* Clean up .. */
	//platform_device_unregister(&rhea_fb_device);
	platform_driver_unregister(&rhea_fb_driver);

	printk(KERN_INFO"BRCM Framebuffer exit OK\n");
}

late_initcall(rhea_fb_init);
module_exit(rhea_fb_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("RHEA FB Driver");
