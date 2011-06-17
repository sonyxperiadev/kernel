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

#if 0
#include <mach/hardware.h>
#include <mach/io.h>
#include <mach/memory.h>
#include <mach/platform.h>
#endif

#include <mach/io.h>

#ifdef CONFIG_ANDROID_POWER
#include <linux/android_power.h>
#endif

#include "bcm2708_fb.h"

struct bcm2708_fb {
	struct resource *fbmem_res;
	phys_addr_t phys_fbbase;
	void __iomem *reg_base;
	void __iomem *fbmem;
	uint32_t ipc_id;
	int irq;
	spinlock_t lock;
	struct semaphore wait_for_irq;
	int base_update_count;
	int rotation;
	struct fb_info fb;
	u32	cmap[16];
#ifdef CONFIG_ANDROID_POWER
	android_early_suspend_t early_suspend;
#endif
};

static inline u32 convert_bitfield(int val, struct fb_bitfield *bf)
{
	unsigned int mask = (1 << bf->length) - 1;

	return (val >> (16 - bf->length) & mask) << bf->offset;
}

static int
bcm2708_fb_setcolreg(unsigned int regno, unsigned int red, unsigned int green,
		 unsigned int blue, unsigned int transp, struct fb_info *info)
{
	struct bcm2708_fb *fb = container_of(info, struct bcm2708_fb, fb);

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

static int bcm2708_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	if((var->rotate & 1) != (info->var.rotate & 1)) {
		if((var->xres != info->var.yres) ||
		   (var->yres != info->var.xres) ||
		   (var->xres_virtual != info->var.yres) ||
		   (var->yres_virtual > info->var.xres * 2) ||
		   (var->yres_virtual < info->var.xres )) {
			bcm2708fb_error("fb_check_var_failed\n");
			return -EINVAL;
		}
	} else {
		if((var->xres != info->var.xres) ||
		   (var->yres != info->var.yres) ||
		   (var->xres_virtual != info->var.xres) ||
		   (var->yres_virtual > info->var.yres * 2) ||
		   (var->yres_virtual < info->var.yres )) {
			bcm2708fb_error("fb_check_var_failed\n");
			return -EINVAL;
		}
	}

	if((var->xoffset != info->var.xoffset) ||
	   (var->bits_per_pixel != info->var.bits_per_pixel) ||
	   (var->grayscale != info->var.grayscale)) {
		bcm2708fb_error("fb_check_var_failed\n");
		return -EINVAL;
	}

	if ((var->yoffset != 0) &&
		(var->yoffset != info->var.yres)) {
		bcm2708fb_error("fb_check_var failed\n");
		bcm2708fb_alert("BRCM fb does not support partial FB updates\n");
		return -EINVAL;
	}

	return 0;
}

static int bcm2708_fb_set_par(struct fb_info *info)
{
	struct bcm2708_fb *fb = container_of(info, struct bcm2708_fb, fb);
	if(fb->rotation != fb->fb.var.rotate) {
		bcm2708fb_warning("Rotation is not supported yet !\n");
		return -EINVAL;
	}

	return 0;
}

static irqreturn_t bcm2708_fb_interrupt(int irq, void *dev_id)
{
	struct bcm2708_fb *fb = (struct bcm2708_fb *)dev_id;
	
	/* Just declare that we got an interrupt */
	up(&fb->wait_for_irq);

	return IRQ_HANDLED;
}

static int bcm2708_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	unsigned long irq_flags;
	int ret;
	struct bcm2708_fb *fb = container_of(info, struct bcm2708_fb, fb);
	uint32_t buff_idx, control, status;
	uint32_t vc_update_count;
	
	/* We are here only if yoffset is '0' or 'yres',
	 * so if yoffset = 0, update first buffer or update second
	 */
	buff_idx = var->yoffset ? 1 : 0;

	spin_lock_irqsave(&fb->lock, irq_flags);

	/* Tell FB to update display .. */
	control = readl(fb->reg_base + FB_CONTROL);
	control &= ~(CTRL_BUFF_IDX_MASK);
	control |= ((buff_idx << CTRL_BUFF_IDX_SHIFT)); 
	writel(control, fb->reg_base + FB_CONTROL);
	fb->base_update_count++;

	/* ring the doorbell */
	ipc_notify_vc_event(fb->irq);
	spin_unlock_irqrestore(&fb->lock, irq_flags);

	ret = down_timeout(&fb->wait_for_irq, HZ*3); /* 3 sec timeout */
	if (ret != 0) {
		bcm2708fb_warning("BRCM fb update timed out, no irq received\n");
		fb->base_update_count--;
		goto out;
	}

	/* poll to read status and timeout after 3 secs */
	status = readl(fb->reg_base + FB_STATUS);
	vc_update_count = status >> STAT_UPDATE_COUNT_SHIFT;
	if(((fb->base_update_count & 0x0000ffff) != vc_update_count)) {
		bcm2708fb_error("Failed: status = (0x%08x), host_update = (0x%08x)\n",
				status, (fb->base_update_count & 0x0000ffff));
		ret = -EAGAIN;
	}

out:
	return ret;
}

#ifdef CONFIG_ANDROID_POWER
static void bcm2708_fb_early_suspend(android_early_suspend_t *h)
{
	struct bcm2708_fb *fb = container_of(h, struct bcm2708_fb, early_suspend);
	bcm2708fb_info("TODO: BRCM fb early suspend ...\n");
}

static void bcm2708_fb_late_resume(android_early_suspend_t *h)
{
	struct bcm2708_fb *fb = container_of(h, struct bcm2708_fb, early_suspend);
	bcm2708fb_info("TODO: BRCM fb late resume ...\n");
}
#endif

static int enable_fb_device(struct bcm2708_fb *fb)
{
	int ret = 0;
	uint32_t status, control;
	uint32_t ready_mask = STAT_EN_MASK | STAT_DISP_MASK;

	/* 
	 * Set width, height, target display, format and num buffers and then
	 * enable framebuffer device
	 */

	writel(FRAMEBUFFER_TARGET_DISPLAY, fb->reg_base + FB_SET_DISPLAY);
	writel(FRAMEBUFFER_WIDTH, fb->reg_base + FB_WIDTH);
	writel(FRAMEBUFFER_HEIGHT, fb->reg_base + FB_HEIGHT);
	writel(FB_FORMAT_RGB565, fb->reg_base + FB_SET_FORMAT);
	writel(FRAMEBUFFER_NUM_BUFFERS, fb->reg_base + FB_SET_NUM_BUFFERS);
	/* Set z-orfer to 0 */
	writel(0, fb->reg_base + FB_TARGET_LAYER);

	/* Write control register bits ...*/
	control = readl(fb->reg_base + FB_CONTROL);
	control &= ~(CTRL_BUFF_IDX_MASK);
	control |= ((1 << CTRL_ENABLE_SHIFT) | (1 << CTRL_DISP_EN_SHIFT)); 
	writel(control, fb->reg_base + FB_CONTROL);

	/* ring the doorbell */
	ipc_notify_vc_event(fb->irq);

	/* Wait for Framebuffer to get enabled .. */
	ret = down_timeout(&fb->wait_for_irq, HZ*3);
	if (ret) {
		bcm2708fb_warning("Wait for fb interrupt timed out ..\n");
		goto out;
	}

	/* Verify we got what we asked for .. */
	if(((status = readl(fb->reg_base + FB_STATUS)) & ready_mask) != ready_mask) {
		bcm2708fb_error("Waiting for fb to come up, status = (0x%08x) ..\n", status);
		ret = -ENODEV;
		goto out;
	}

	bcm2708fb_debug("Got status = 0x%08x\n",status);

	fb->base_update_count++;
	fb->phys_fbbase = readl(fb->reg_base + FB_GET_BUFFER_PHYS);
	if (!fb->phys_fbbase) {
		bcm2708fb_error("FB phys memory pointer is NULL\n");
		ret = -ENODEV;
		goto out;
	}

	/* Convert VC phys addr to Kernel phys addr */
	fb->phys_fbbase = __VC_BUS_TO_ARM_PHYS_ADDR(fb->phys_fbbase);

out:
	return ret;
}

static int disable_fb_device(struct bcm2708_fb *fb)
{
	uint32_t control, status;
	uint32_t disable_mask = (STAT_EN_MASK | STAT_DISP_MASK);
	int ret;

	/* Write control register bits ...*/
	control = readl(fb->reg_base + FB_CONTROL);
	control &= ~CTRL_ENABLE_MASK; 
	writel(control, fb->reg_base + FB_CONTROL);

	ipc_notify_vc_event(fb->irq);
	ret = down_timeout(&fb->wait_for_irq, HZ*3);
	if (ret != 0) {
		bcm2708fb_error("Disable BCM2708 FB device timed out \n");
		goto out;
	}

	if ((status = readl(fb->reg_base + FB_STATUS)) & disable_mask) {
		bcm2708fb_error("BCM2708 FB device not disabled, but we got an IRQ\n");
		ret = -EIO;
		goto out;
	}

	bcm2708fb_info("BCM278 FB device disabled successfully\n");
out:
	return ret;
}


static struct fb_ops bcm2708_fb_ops = {
	.owner          = THIS_MODULE,
	.fb_check_var   = bcm2708_fb_check_var,
	.fb_set_par     = bcm2708_fb_set_par,
	.fb_setcolreg   = bcm2708_fb_setcolreg,
	.fb_pan_display = bcm2708_fb_pan_display,
	.fb_fillrect    = cfb_fillrect,
	.fb_copyarea    = cfb_copyarea,
	.fb_imageblit   = cfb_imageblit,
};

static int bcm2708_fb_probe(struct platform_device *pdev)
{
	int ret = -ENXIO;
	struct resource *r;
	struct bcm2708_fb *fb;
	size_t framesize;
	uint32_t width, height;

	fb = kzalloc(sizeof(struct bcm2708_fb), GFP_KERNEL);
	if (fb == NULL) {
		bcm2708fb_error("Unable to allocate framebuffer structure\n");
		ret = -ENOMEM;
		goto err_fb_alloc_failed;
	}

#ifdef DEBUG_FRAMEBUFFER
	global_hack = fb;
#endif

	spin_lock_init(&fb->lock);
	sema_init(&fb->wait_for_irq, 0);
	platform_set_drvdata(pdev, fb);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(r == NULL) {
		bcm2708fb_error("Unable to get framebuffer memory resource\n");
		ret = -ENODEV;
		goto err_no_io_base;
	}

	bcm2708fb_info("FB registers start-end (0x%08x)-(0x%08x)\n", r->start, r->end);

	/* 
	 * IPC driver puts virtual address here so we dont need to
	 * use the __io_address()
	 */
	fb->reg_base = (void __iomem *)r->start;

	fb->irq = platform_get_irq(pdev, 0);
	if(fb->irq < 0) {
		bcm2708fb_error("Unable to get framebuffer irq resource\n");
		ret = -ENODEV;
		goto err_no_irq;
	}
	
	ret = request_irq(fb->irq, bcm2708_fb_interrupt, IRQF_DISABLED,
				"bcm2708 fb interrupt", (void *)fb);
	if (ret < 0) {
		bcm2708fb_error("Unable to register Interrupt for bcm2708 FB\n");
		goto err_no_irq;
	}

	ret = enable_fb_device(fb);
	if (ret) {
		bcm2708fb_error("Failed to enable FB device\n");
		goto err_enable_fb_failed;
	}
	
	/* Now we should get correct width and height for this display .. */
	width = readl(fb->reg_base + FB_WIDTH);
	height = readl(fb->reg_base + FB_HEIGHT);

	fb->fb.fbops		= &bcm2708_fb_ops;
	fb->fb.flags		= FBINFO_FLAG_DEFAULT;
	fb->fb.pseudo_palette	= fb->cmap;
	fb->fb.fix.type		= FB_TYPE_PACKED_PIXELS;
	fb->fb.fix.visual	= FB_VISUAL_TRUECOLOR;
	fb->fb.fix.line_length	= width * 2;
	fb->fb.fix.accel	= FB_ACCEL_NONE;
	fb->fb.fix.ypanstep	= 1;

	fb->fb.var.xres		= width;
	fb->fb.var.yres		= height;
	fb->fb.var.xres_virtual	= width;
	fb->fb.var.yres_virtual	= height * 2;
	fb->fb.var.bits_per_pixel = 16;
	fb->fb.var.activate	= FB_ACTIVATE_NOW;
	fb->fb.var.height	= height;
	fb->fb.var.width	= width;

	fb->fb.var.red.offset = 11;
	fb->fb.var.red.length = 5;
	fb->fb.var.green.offset = 5;
	fb->fb.var.green.length = 6;
	fb->fb.var.blue.offset = 0;
	fb->fb.var.blue.length = 5;

	framesize = width * height * 2 * 2;

	fb->fb.fix.smem_start = fb->phys_fbbase;
	fb->fb.fix.smem_len = framesize;

	bcm2708fb_debug("Requesting and mapping resource (0x%08x)-(0x%08x)\n", r->start, r->end);
	fb->fbmem_res = request_mem_region(fb->phys_fbbase, framesize, "Framebuffer Memory");
	if (fb->fbmem_res == NULL) {
		ret = -ENOMEM;
		bcm2708fb_error("Unable to get fb memory resource\n");
		goto err_get_fbmem_resource_failed;

	}

	fb->fb.screen_base = ioremap(fb->fbmem_res->start, resource_size(fb->fbmem_res)); 
	if (fb->fb.screen_base == NULL) {
		ret = -ENOMEM;
		bcm2708fb_error("Unable to ioremap fb memory resource\n");
		goto err_ioremap_fbmem_failed;
	}

	bcm2708fb_info("Framebuffer starts at phys[0x%08x], and virt[0x%08x]\n",
			fb->phys_fbbase, (uint32_t)fb->fb.screen_base);

	ret = fb_set_var(&fb->fb, &fb->fb.var);
	if (ret) {
		bcm2708fb_error("fb_set_var failed\n");
		goto err_set_var_failed;
	}

	/* Paint it black (assuming default fb contents are all zero) */
	bcm2708_fb_pan_display(&fb->fb.var, &fb->fb);

	ret = register_framebuffer(&fb->fb);
	if (ret) {
		bcm2708fb_error("Framebuffer registration failed\n");
		goto err_fb_register_failed;
	}

	bcm2708fb_info("BCM2708 Framebuffer probe successfull\n");

#ifdef CONFIG_ANDROID_POWER
	fb->early_suspend.suspend = bcm2708_fb_early_suspend;
	fb->early_suspend.resume = bcm2708_fb_late_resume;
	android_register_early_suspend(&fb->early_suspend);
#endif

	return 0;

err_fb_register_failed:
err_set_var_failed:
	iounmap(fb->fb.screen_base);
err_ioremap_fbmem_failed:
	release_region(fb->fbmem_res->start, resource_size(fb->fbmem_res));
err_get_fbmem_resource_failed:
	disable_fb_device(fb);
err_enable_fb_failed:
	free_irq(fb->irq, (void *)fb);
err_no_irq:
err_no_io_base:
	kfree(fb);
err_fb_alloc_failed:
	bcm2708fb_alert("BCM2708 Framebuffer probe FAILED !!\n");
	return ret;
}

static int __devexit bcm2708_fb_remove(struct platform_device *pdev)
{
	size_t framesize;
	struct bcm2708_fb *fb = platform_get_drvdata(pdev);
	
	framesize = fb->fb.var.xres_virtual * fb->fb.var.yres_virtual * 2;

#ifdef CONFIG_ANDROID_POWER
        android_unregister_early_suspend(&fb->early_suspend);
#endif
	unregister_framebuffer(&fb->fb);
	iounmap(fb->fb.screen_base);
	release_region(fb->fbmem_res->start, resource_size(fb->fbmem_res));
	disable_fb_device(fb);
	kfree(fb);
	bcm2708fb_info("BCM2708 FB removed !!\n");
	return 0;
}

static struct platform_driver bcm2708_fb_driver = {
	.probe		= bcm2708_fb_probe,
	.remove		= __devexit_p(bcm2708_fb_remove),
	.driver = {
		.name = "bcm2835_FBUF"
	}
};

static int __init bcm2708_fb_init(void)
{
	int ret;

	/* FIXME:ssp check if this device exists ?? */
	ret = platform_driver_register(&bcm2708_fb_driver);
	if (ret)
		printk(KERN_ERR"%s : Unable to register BRCM battery driver\n", __func__);

	printk(KERN_INFO"BRCM Framebuffer Init %s !\n", ret ? "FAILED" : "OK");

	return ret;
}

static void __exit bcm2708_fb_exit(void)
{
	/* Clean up .. */
	platform_driver_unregister(&bcm2708_fb_driver);

	printk(KERN_INFO"BRCM Framebuffer exit OK\n");
}

module_init(bcm2708_fb_init);
module_exit(bcm2708_fb_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("BCM2708 FB Driver");
