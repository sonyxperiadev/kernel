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

#include <video/vceb_fb.h>

#ifdef CONFIG_ANDROID_POWER
#include <linux/android_power.h>
#endif

/*
 * Enable this if you want to have a framebuffer refresh thread.
 * This can be used in case of framebuffer console
 */

/* define HAVE_FB_REFRESH_THREAD */

#ifdef CONFIG_MACH_MAPPHONE
#define VCEB_SCREEN1_WIDTH	480
#define VCEB_SCREEN1_HEIGHT	854
#else
#define VCEB_SCREEN1_WIDTH	800
#define VCEB_SCREEN1_HEIGHT	480
#endif

#define VCEB_SCREEN2_WIDTH	640
#define VCEB_SCREEN2_HEIGHT	480

#define VCEB_FPS              60

static struct vceb_fb *fb_global_hack;

struct vceb_screen_info {
	uint32_t screen_num;
	uint32_t keep_vmcs_res;
	struct proc_dir_entry *vmcsfb_keepres;
	atomic_t f_count; /* vceb fb global ref count */
	atomic_t vmcs_fb_opened; /* vmcs fb open/release flag */
	struct fb_info fb;
	u32	cmap[16];
	void *data;

	struct vmcs_fb_ops *ops;
	void *callback_param;
};

struct vceb_fb {
	struct device *dev;
#ifdef CONFIG_ANDROID_POWER
	android_early_suspend_t early_suspend;
#endif
	struct vceb_screen_info *screen1;
	struct vceb_screen_info *screen2;
};

static inline struct vceb_screen_info *to_screen_info(struct fb_info *info)
{
	return container_of(info, struct vceb_screen_info, fb);
}

static inline u32 convert_bitfield(int val, struct fb_bitfield *bf)
{
	unsigned int mask = (1 << bf->length) - 1;

	return (val >> (16 - bf->length) & mask) << bf->offset;
}

static int
vceb_fb_setcolreg(unsigned int regno, unsigned int red, unsigned int green,
		 unsigned int blue, unsigned int transp, struct fb_info *info)
{
	struct vceb_screen_info *scrn_info = to_screen_info(info);
	u32 *cmap = scrn_info->cmap;

	if (regno < 16) {
		cmap[regno] = convert_bitfield(transp, &info->var.transp) |
			convert_bitfield(blue, &info->var.blue) |
			convert_bitfield(green, &info->var.green) |
			convert_bitfield(red, &info->var.red);
		return 0;
	} else {
		return 1;
	}
}

static int
vceb_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct vceb_screen_info *screen;
	uint32_t width, height;

	screen = to_screen_info(info);
	width = screen->screen_num ? VCEB_SCREEN2_WIDTH : VCEB_SCREEN1_WIDTH;
	height = screen->screen_num ? VCEB_SCREEN2_HEIGHT : VCEB_SCREEN1_HEIGHT;

	/*  check for rotation */
	if ((var->rotate & 1) != (info->var.rotate & 1)) {

		/* No rotation on screen 2 (HDMI) */
		if (screen->screen_num) {
			printk(KERN_WARNING"fb_check_var() failed at %s:%d\n",
					__FILE__, __LINE__);
			return -EINVAL;
		}

		if ((var->xres != info->var.yres) ||
		   (var->yres != info->var.xres) ||
		   (var->xres_virtual != info->var.yres) ||
		   (var->yres_virtual > info->var.xres * 2) ||
		   (var->yres_virtual < info->var.xres)) {

			/* (xres X yres) = (yres X xres) */
			var->xres = info->var.yres;
			var->yres = info->var.xres;
			var->xres_virtual = info->var.yres;
			var->yres_virtual = info->var.xres * 2;
			if (var->yoffset)
				var->yoffset = 0;

			printk(KERN_INFO"vceb_fb: VAR info rotated\n");
			return 0;
		}
	}

	if ((var->yres_virtual > info->var.yres * 2) ||
			(var->yres_virtual < info->var.yres)) {

		printk(KERN_WARNING"fb_check_var() failed at %s:%d\n",
					__FILE__, __LINE__);
		return -EINVAL;

	} else if ((var->xres > width) && (var->xres > height)) {

		printk(KERN_WARNING"fb_check_var() failed at %s:%d\n",
					__FILE__, __LINE__);
		return -EINVAL;
	}

	if ((var->xoffset != info->var.xoffset) ||
	   (var->bits_per_pixel != info->var.bits_per_pixel) ||
	   (var->grayscale != info->var.grayscale)) {
		printk(KERN_WARNING"fb_check_var() failed at %s:%d\n",
					__FILE__, __LINE__);
		return -EINVAL;
	}
	return 0;
}

static int vceb_fb_set_par(struct fb_info *info)
{
	struct vceb_screen_info *scrn_info = to_screen_info(info);

	if (scrn_info->screen_num == 2) /* HDMI screen */
		return 0;

	if (info->var.rotate & 3)
		info->fix.line_length = info->var.xres * 2;

	return 0;
}

static int
vceb_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	int success = 0 ;
	struct vceb_screen_info *scrn = to_screen_info(info);
	uint32_t offset = scrn->fb.var.xres * 2 * var->yoffset;
	void *data = (void *)(scrn->fb.screen_base + offset);

	if (!atomic_read(&scrn->vmcs_fb_opened)) {
		if (scrn->ops) {
			/* Try to open vmcs_fb */
			if (scrn->ops->open) {
				success = scrn->ops->open(scrn->callback_param,
						scrn->screen_num, info);
				if (success == 0) {
					atomic_set(&scrn->vmcs_fb_opened, 1);
				} else if (success != -EAGAIN) {
				/* If -EAGAIN is returned, we automatically fall
				 * back to VCEB updates, and try to open on next
				 * update
				 */
					return success;
				}
			}
		}
	}

	/* update only if we successfully opened vmcs fb */
	if (atomic_read(&scrn->vmcs_fb_opened)) {
		/* ops must be registered if vmcs_fb is opened */
		BUG_ON(scrn->ops == NULL);
		if (scrn->ops->update)
			success = scrn->ops->update(data,
					scrn->callback_param,
					scrn->screen_num,
					info);
	} else if (scrn->screen_num == 0) {
		/* Block until data is updated */
		vceb_framebuffer_overlay_set(data, VCEB_RBG_FORMAT_RGB565, 0,
				VCEB_SCREEN1_WIDTH, VCEB_SCREEN1_HEIGHT,
				VCEB_ALIGN_CENTRE);
	}


   return success;
}

#ifdef CONFIG_ANDROID_POWER
static void vceb_fb_early_suspend(android_early_suspend_t *h)
{
}

static void vceb_fb_late_resume(android_early_suspend_t *h)
{
}
#endif

static int vceb_fb_open(struct fb_info *info, int user)
{
	struct vceb_screen_info *scrn = to_screen_info(info);
	int ret = 0;

	/* Stop fbcon from opening our fb device */
	if (user == 0) {
		ret = -ENODEV;
		goto out;
	}

	if (!atomic_read(&scrn->f_count) &&
			!atomic_read(&scrn->vmcs_fb_opened)) {
		/* Allocate vmcs_fb_resources here
		 * return failure if vmcs_fb not registered */

		if (scrn->ops) {
			if (scrn->ops->open) {
				ret = scrn->ops->open(scrn->callback_param,
						scrn->screen_num, info);
				if (ret == 0)
					atomic_set(&scrn->vmcs_fb_opened, 1);
			}
		}
	}

	/* -EAGAIN is returned if dispmanx/gencmd service are not
	 * ready, which is when VMCS image is not downloaded/connected.
	 * So, we fall back to VCEB for current update, and let
	 * it try again next time.
	 *
	 * NOTE: This will only work if VCEB is still running on
	 * VideoCore
	 */
out:
	if ((ret == 0) || (ret == -EAGAIN)) {
		atomic_inc(&scrn->f_count);
		ret = 0;
	} else {
		printk(KERN_WARNING"%s : FB open failed\n", __func__);
	}

	return ret;
}

static int vceb_fb_release(struct fb_info *info, int user)
{
	struct vceb_screen_info *scrn = to_screen_info(info);
	int ret = 0;

	if (atomic_dec_and_test(&scrn->f_count)) {

		if (scrn->ops && atomic_read(&scrn->vmcs_fb_opened) && !scrn->keep_vmcs_res) {
			if (scrn->ops->release) {
				scrn->ops->release(scrn->callback_param,
							scrn->screen_num);
				atomic_set(&scrn->vmcs_fb_opened, 0);
			}
		}
	}

	return ret;
}

static struct fb_ops vceb_fb_ops = {
	.owner          = THIS_MODULE,
	.fb_open	= vceb_fb_open,
	.fb_release	= vceb_fb_release,
	.fb_check_var   = vceb_fb_check_var,
	.fb_set_par     = vceb_fb_set_par,
	.fb_setcolreg   = vceb_fb_setcolreg,
	.fb_pan_display = vceb_fb_pan_display,
	.fb_fillrect    = cfb_fillrect,
	.fb_copyarea    = cfb_copyarea,
	.fb_imageblit   = cfb_imageblit,
};

static int init_fb_screen(struct vceb_fb *fb, uint32_t screen)
{
	int success = -1;
	struct vceb_screen_info *scrn_info;
	dma_addr_t fbpaddr;
	size_t framesize;
	uint32_t width, height;

	if (screen != 0 && screen != 2)
			return success;

	scrn_info = kzalloc(sizeof(*scrn_info), GFP_KERNEL);
	if (!scrn_info)
			return -ENOMEM;

	if (screen)
		fb->screen2 = scrn_info;
	else
		fb->screen1 = scrn_info;

	scrn_info->screen_num = screen;

	width = screen ? VCEB_SCREEN2_WIDTH : VCEB_SCREEN1_WIDTH;
	height = screen ? VCEB_SCREEN2_HEIGHT : VCEB_SCREEN1_HEIGHT;

	atomic_set(&scrn_info->f_count, 0);
	atomic_set(&scrn_info->vmcs_fb_opened, 0);
	scrn_info->keep_vmcs_res = 0;
	scrn_info->vmcsfb_keepres = NULL;
	scrn_info->data = (void *)fb;
	scrn_info->fb.fbops = &vceb_fb_ops;
	scrn_info->fb.flags = FBINFO_FLAG_DEFAULT;
	scrn_info->fb.pseudo_palette = scrn_info->cmap;
	scrn_info->fb.fix.type = FB_TYPE_PACKED_PIXELS;
	scrn_info->fb.fix.visual = FB_VISUAL_TRUECOLOR;
	scrn_info->fb.fix.line_length = width * 2;
	scrn_info->fb.fix.accel	= FB_ACCEL_NONE;
	scrn_info->fb.fix.ypanstep = 1;

	scrn_info->fb.var.xres = width;
	scrn_info->fb.var.yres = height;
	scrn_info->fb.var.xres_virtual = width;
	scrn_info->fb.var.rotate = 0;

	if (!screen)
		scrn_info->fb.var.yres_virtual	= height * 2;
	else
		scrn_info->fb.var.yres_virtual	= height;

	scrn_info->fb.var.bits_per_pixel = 16;
	scrn_info->fb.var.activate = FB_ACTIVATE_NOW;
	scrn_info->fb.var.height = 90;
	scrn_info->fb.var.width	= 90;
	scrn_info->fb.var.red.offset = 11;
	scrn_info->fb.var.red.length = 5;
	scrn_info->fb.var.green.offset = 5;
	scrn_info->fb.var.green.length = 6;
	scrn_info->fb.var.blue.offset = 0;
	scrn_info->fb.var.blue.length = 5;

	if (!screen)
		/* Double the size of framebuffer */
		framesize = width * height * 2 * 2;
	else
		framesize = width * height * 2;

	scrn_info->fb.screen_base = dma_alloc_writecombine(fb->dev, framesize,
							&fbpaddr, GFP_KERNEL);

	printk(KERN_INFO "allocating frame buffer %d * %d, got %p\n",
			width, height, scrn_info->fb.screen_base);

	if (scrn_info->fb.screen_base == 0)
		return -ENOMEM;

	scrn_info->fb.fix.smem_start = fbpaddr;
	scrn_info->fb.fix.smem_len = framesize;

	success = fb_set_var(&scrn_info->fb, &scrn_info->fb.var);

	if (success) {
		printk(KERN_ERR "vceb_fb - err_fb_set_var_failed\n");
		dma_free_writecombine(fb->dev, framesize,
						scrn_info->fb.screen_base,
						scrn_info->fb.fix.smem_start);
	}

	return success;
}

static void delete_fb_screen(struct vceb_fb *fb, uint32_t screen)
{
	struct vceb_screen_info *scrn_info;
	size_t framesize;
	uint32_t width, height;

	scrn_info = screen ? fb->screen2 : fb->screen1;

	if (!scrn_info) {
		printk(KERN_WARNING"Trying to delete un-initialised screen\n");
		return;
	}

	width = screen ? VCEB_SCREEN2_WIDTH : VCEB_SCREEN1_WIDTH;
	height = screen ? VCEB_SCREEN2_HEIGHT : VCEB_SCREEN1_HEIGHT;
	if (!screen)
		framesize = width * height * 2 * 2;
	else
		framesize = width * height * 2;

	dma_free_writecombine(fb->dev, framesize, scrn_info->fb.screen_base,
						scrn_info->fb.fix.smem_start);

	kfree(scrn_info);
}

static int
vmcs_keepres_dummy_read_proc(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
 	int len = 0;


	if (offset > 0) {
		*eof = 1;
		return 0;
	}

	//is there anything to return for this func?
	//not at the moment!

	*eof = 1;

	return len;

}

static int
vmcs_keepres_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char keepres_str[10];
	unsigned long keepres_val;
	struct proc_dir_entry *proc_ent;
	struct vceb_screen_info *scrn;

	proc_ent = PDE(file->f_path.dentry->d_inode);

	if (proc_ent == fb_global_hack->screen1->vmcsfb_keepres)
		scrn = fb_global_hack->screen1;
	else
		scrn = fb_global_hack->screen2;

	memset(keepres_str, 0, 10);
	if (copy_from_user(keepres_str, buffer, 1))
		return -EFAULT;

	keepres_val = simple_strtoul(keepres_str, NULL, 10);
	if (keepres_val > 0)
		keepres_val = 1;

	/* Has the value changed from last time ? */
	if (keepres_val != scrn->keep_vmcs_res) {
		scrn->keep_vmcs_res = keepres_val;

		/* Check if someone has this device open */
		if (atomic_read(&scrn->f_count) == 0) {
			/* release resources if keepres is zero */
			if (scrn->ops && atomic_read(&scrn->vmcs_fb_opened) && !scrn->keep_vmcs_res) {
				if (scrn->ops->release) {
					scrn->ops->release(scrn->callback_param,
							scrn->screen_num);
					atomic_set(&scrn->vmcs_fb_opened, 0);
				}
			}
		} else {
			printk(KERN_WARNING"******************************************************\n");
			printk(KERN_WARNING"There are (%d) processes holding FB%d open\n",
					atomic_read(&scrn->f_count), scrn->screen_num);
			printk(KERN_WARNING"VMCS FB resources will not be released this time\n");
			printk(KERN_WARNING"******************************************************\n");
		}

	}

	return count;
}


static int
vceb_fb_add_framebuffer(struct vceb_fb *fb, uint32_t screen)
{
	struct vceb_screen_info *scrn_info;
	struct proc_dir_entry *proc;
	int success = -1;
	char proc_name[20];

	if (screen != 0 && screen != 2) {
		printk(KERN_WARNING"%s: screen number [%i] is not allowed\n",
							__func__, screen);
		return success;
	}

	success = init_fb_screen(fb, screen);
	if (success)
		goto err_init_fb;

	scrn_info = screen ? fb->screen2 : fb->screen1;

	sprintf(proc_name, "vmcsfb%d_keepres", screen);

	proc = create_proc_entry(proc_name, 0666, NULL);
	if (proc == NULL) {
		printk(KERN_ALERT"Failed to create proc entry (%s)\n", proc_name);
		printk(KERN_ALERT"vmcsfb resources will always be released after use\n");
	} else {
		proc->read_proc =  &vmcs_keepres_dummy_read_proc;
		proc->write_proc = &vmcs_keepres_write_proc;
		scrn_info->vmcsfb_keepres = proc;
	}

	success = register_framebuffer(&scrn_info->fb);
	if (success)
		goto err_register_framebuffer;

	return success;

err_register_framebuffer:
	printk(KERN_ERR"vceb registration failed for screen [%i]\n", screen);
	delete_fb_screen(fb, screen);
err_init_fb:
	return success;
}

static void vceb_fb_remove_framebuffer(struct vceb_fb *fb, uint32_t screen)
{
	struct vceb_screen_info *scrn_info;
	char proc_name[20];

	scrn_info = screen ? fb->screen2 : fb->screen1;
	
	if (scrn_info) {
	        sprintf(proc_name, "vmcsfb%d_keepres", scrn_info->screen_num);
	        remove_proc_entry(proc_name, NULL);
		unregister_framebuffer(&scrn_info->fb);
		delete_fb_screen(fb, screen);
	}

	return;
}

static int vceb_fb_probe(struct platform_device *pdev)
{
	int ret;
	struct vceb_fb *fb;

	printk(KERN_ERR "vceb_fb_probe\n");

	fb = kzalloc(sizeof(*fb), GFP_KERNEL);

	if (fb == NULL) {
		ret = -ENOMEM;
		printk(KERN_ERR "vceb_fb_probe - err_fb_alloc_failed\n");
		goto err_fb_alloc_failed;
	}

	platform_set_drvdata(pdev, fb);
	fb->dev = &pdev->dev;

	/* store a nasty global pointer to this data */
	fb_global_hack = fb;

	/*
	 * VCEB init - no_reset
	 */

	vceb_initialise(0);

	/* enable overlay, if this fails, FB is not going to work at all */
	ret = vceb_framebuffer_overlay_enable(1);
	if (ret) {
		printk(KERN_ERR"VCEB overlay enable failed\n");
		goto err_vceb_init;
	}

	ret = vceb_fb_add_framebuffer(fb, 0);
	if (ret)
		goto err_vceb_add_framebuffer;

#ifdef CONFIG_ANDROID_POWER
	fb->early_suspend.suspend = vceb_fb_early_suspend;
	fb->early_suspend.resume = vceb_fb_late_resume;
	android_register_early_suspend(&fb->early_suspend);
#endif


	printk(KERN_ERR "vceb_fb_probe - succeeded!\n");

	return 0;

err_vceb_add_framebuffer:
   printk(KERN_ERR "vceb_fb - err_add_framebuffer_failed\n");
err_vceb_init:
	kfree(fb);
err_fb_alloc_failed:
	return ret;
}

static int vceb_fb_remove(struct platform_device *pdev)
{
	struct vceb_fb *fb = platform_get_drvdata(pdev);

#ifdef CONFIG_ANDROID_POWER
	android_unregister_early_suspend(&fb->early_suspend);
#endif

	/* Remove both screens (framebuffers) */
	vceb_fb_remove_framebuffer(fb, 0);
	vceb_fb_remove_framebuffer(fb, 2);

	kfree(fb);
	return 0;
}

static struct platform_driver vceb_fb_driver = {
	.probe		= vceb_fb_probe,
	.remove		= vceb_fb_remove,
	.driver = {
		.name = "vceb_fb"
	}
};

static int __init vceb_fb_init(void)
{
	printk(KERN_INFO"vceb_fb_init\n");

	return platform_driver_register(&vceb_fb_driver);
}

static void __exit vceb_fb_exit(void)
{
	printk(KERN_INFO"vceb_fb_exit\n");

	platform_driver_unregister(&vceb_fb_driver);
}

/* Function to register framebuffer for new screen */
int32_t vceb_fb_add_screen(uint32_t screen)
{
	if (screen != 0 && screen != 2) {
		printk(KERN_ERR"%s: Cannnot add vceb screen [%i]\n",
						__func__, screen);
		return -ENODEV;
	}

	return vceb_fb_add_framebuffer(fb_global_hack, screen);
}
EXPORT_SYMBOL(vceb_fb_add_screen);

void vceb_fb_remove_screen(uint32_t screen)
{
	if (screen != 0 && screen != 2)
		printk(KERN_ERR"%s: Cannnot remove vceb screen [%i]\n",
						__func__, screen);

	vceb_fb_remove_framebuffer(fb_global_hack, screen);
}
EXPORT_SYMBOL(vceb_fb_remove_screen);

/*
 * function to register a callback that fires whenever framebuffer is updated
 */
int vceb_fb_register_callbacks(uint32_t screen, struct vmcs_fb_ops *ops,
				void *callback_param)
{
	int32_t success = 0;
	struct vceb_screen_info *scrn_info;

	printk(KERN_ERR "vceb_fb_register_callbacks\n");

	if (screen != 0 && screen != 2)
		return -ENOENT;

	scrn_info = screen ? fb_global_hack->screen2 : fb_global_hack->screen1;

	if (!scrn_info) {
		printk(KERN_ERR"FB isn't registered on screen [%i]\n", screen);
		return -ENOENT;
	}

	/* If someone has already opened this device (e.g. fbcon),
	 * make sure we call ops->open(), and the lock should
	 * protect us from update() being called before this open()
	 */

	if (atomic_read(&scrn_info->f_count) &&
			!atomic_read(&scrn_info->vmcs_fb_opened) &&
			(ops != NULL)) {
		success = ops->open(callback_param,
				scrn_info->screen_num, &scrn_info->fb);

		/* If failed to open with -EAGAIN, VC might not be ready yet,
		 * we defer ops->open() call to next fb_open/fb_pan_display.
		 */

		if (success == 0)
			atomic_set(&scrn_info->vmcs_fb_opened, 1);
		else if (success == -EAGAIN)
			success = 0; /* we don't want vmcs_fb driver to fail */
	}

	/* This is unregister callback, and if we already have the device open,
	 * explicitly release vmcs_fb resources before unregister
	 */
	if (atomic_read(&scrn_info->vmcs_fb_opened) && (ops == NULL)) {
		scrn_info->ops->release(scrn_info->callback_param,
							scrn_info->screen_num);
		scrn_info->keep_vmcs_res = 0;
		atomic_set(&scrn_info->vmcs_fb_opened, 0);
	}

	/* register/unregister vmcs_fb ops */
	scrn_info->ops = ops;
	scrn_info->callback_param = callback_param;

	/* If we registered vmcs_fb callback successfully,
	 * send an update to show current framebuffer on the screen
	 */
	/* FIXME: For now, we only send current fb update here if screen is 0,
	 * i.e. screen is main LCD
	 */
	if ((screen == 0) && (ops != NULL))
		vceb_fb_pan_display(&scrn_info->fb.var, &scrn_info->fb);

	printk(KERN_ERR "vceb_fb_""%s""_callbacks OK!\n",
			scrn_info->ops ? "register" : "unregister");

	return success;
}
EXPORT_SYMBOL(vceb_fb_register_callbacks);

void vceb_fb_bus_connected(uint32_t keep_vmcsfb)
{
	/* only update primary screen */
	struct vceb_screen_info *scrn_info = fb_global_hack->screen1;

	if ((scrn_info->ops != NULL)) {
		vceb_fb_pan_display(&scrn_info->fb.var, &scrn_info->fb);

		/* set keep_res value to keep_vmcsfb */
		if (scrn_info->vmcsfb_keepres)
			scrn_info->keep_vmcs_res = keep_vmcsfb;

		/* do not keep resources, if f_count and keep_vmcs_res are 0 */
		if (!atomic_read(&scrn_info->f_count) && !(scrn_info->keep_vmcs_res))
			scrn_info->ops->release(scrn_info->callback_param,
	    	    				scrn_info->screen_num);
	} else {
		printk(KERN_ALERT"%s() called w/o vmcsfb ops registered\n", __func__);
	}

}
EXPORT_SYMBOL(vceb_fb_bus_connected);

module_init(vceb_fb_init);
module_exit(vceb_fb_exit);
