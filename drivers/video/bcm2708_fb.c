/*
 *  linux/drivers/video/bcm2708_fb.c
 *
 * Copyright (C) 2010 Broadcom
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 *  Broadcom simple framebuffer driver
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>

#include <mach/platform.h>
#include <mach/vcio.h>

#include <asm/sizes.h>
#include <asm/io.h>

/* This is limited to 16 characters when displayed by X startup */
static const char *bcm2708_name = "BCM2708 FB";

#define DRIVER_NAME "bcm2708_fb"

/* this data structure describes each frame buffer device we find */

struct bcm2708_fb {
	struct fb_info		fb;
	struct platform_device	*dev;
	void __iomem		*regs;
	u32			cmap[16];
};

#define to_bcm2708(info)	container_of(info, struct bcm2708_fb, fb)

static int
bcm2708_fb_set_bitfields(struct bcm2708_fb *fb, struct fb_var_screeninfo *var)
{
	int ret = 0;

	memset(&var->transp, 0, sizeof(var->transp));

	var->red.msb_right = 0;
	var->green.msb_right = 0;
	var->blue.msb_right = 0;

	switch (var->bits_per_pixel) {
	case 1:
	case 2:
	case 4:
	case 8:
		var->red.length		= var->bits_per_pixel;
		var->red.offset		= 0;
		var->green.length	= var->bits_per_pixel;
		var->green.offset	= 0;
		var->blue.length	= var->bits_per_pixel;
		var->blue.offset	= 0;
		break;
	case 16:
		var->red.length = 5;
		var->blue.length = 5;
		/*
		 * Green length can be 5 or 6 depending whether
		 * we're operating in RGB555 or RGB565 mode.
		 */
		if (var->green.length != 5 && var->green.length != 6)
			var->green.length = 6;
		break;
	case 32:
		var->red.length		= 8;
		var->green.length	= 8;
		var->blue.length	= 8;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	/*
	 * >= 16bpp displays have separate colour component bitfields
	 * encoded in the pixel data.  Calculate their position from
	 * the bitfield length defined above.
	 */
	if (ret == 0 && var->bits_per_pixel >= 16) {
		var->blue.offset = 0;
		var->green.offset = var->blue.offset + var->blue.length;
		var->red.offset = var->green.offset + var->green.length;
	}

	return ret;
}

static int bcm2708_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{

	/* Usually we check for bounds here, but since we are not doing anything
	 * right now, always return success
	 */

	return 0;
}

static int bcm2708_fb_set_par(struct fb_info *info)
{
	struct bcm2708_fb *fb = to_bcm2708(info);

	fb->fb.fix.line_length = fb->fb.var.xres_virtual *
				 fb->fb.var.bits_per_pixel / 8;

	if (fb->fb.var.bits_per_pixel <= 8)
		fb->fb.fix.visual = FB_VISUAL_PSEUDOCOLOR;
	else
		fb->fb.fix.visual = FB_VISUAL_TRUECOLOR;
        
        bcm_mailbox_write(MBOX_CHAN_FB,
                          fb->fb.fix.smem_start +
                          fb->fb.var.yoffset * fb->fb.fix.line_length);
        
	printk(KERN_INFO "BCM2708FB: start = 0x%08x\n",
               (unsigned)fb->fb.fix.smem_start +
               fb->fb.var.yoffset * fb->fb.fix.line_length);

	return 0;
}

static inline u32 convert_bitfield(int val, struct fb_bitfield *bf)
{
	unsigned int mask = (1 << bf->length) - 1;

	return (val >> (16 - bf->length) & mask) << bf->offset;
}

static int bcm2708_fb_setcolreg(unsigned int regno, unsigned int red, unsigned int green,
		 unsigned int blue, unsigned int transp, struct fb_info *info)
{
	struct bcm2708_fb *fb = to_bcm2708(info);

	if (regno < 16)
		fb->cmap[regno] = convert_bitfield(transp, &fb->fb.var.transp) |
				  convert_bitfield(blue, &fb->fb.var.blue) |
				  convert_bitfield(green, &fb->fb.var.green) |
				  convert_bitfield(red, &fb->fb.var.red);

	return regno > 255;
}

static int bcm2708_fb_blank(int blank_mode, struct fb_info *info)
{
	return -1;
}

static int bcm2708_fb_mmap(struct fb_info *info,
		       struct vm_area_struct *vma)
{
	struct bcm2708_fb *fb = to_bcm2708(info);
	unsigned long len, off = vma->vm_pgoff << PAGE_SHIFT;
	int ret = -EINVAL;

	len = info->fix.smem_len;

	if (off <= len && vma->vm_end - vma->vm_start <= len - off)
		ret = dma_mmap_writecombine(&fb->dev->dev, vma,
					    fb->fb.screen_base,
					    fb->fb.fix.smem_start,
					    fb->fb.fix.smem_len);

	return ret;
}

static struct fb_ops bcm2708_fb_ops = {
	.owner		= THIS_MODULE,
	.fb_check_var	= bcm2708_fb_check_var,
	.fb_set_par	= bcm2708_fb_set_par,
	.fb_setcolreg	= bcm2708_fb_setcolreg,
	.fb_blank	= bcm2708_fb_blank,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
	.fb_mmap	= bcm2708_fb_mmap,
};

static int bcm2708_fb_register(struct bcm2708_fb *fb)
{
	dma_addr_t dma;

	int ret;
	size_t framesize;
        
	fb->fb.fbops		= &bcm2708_fb_ops;
	fb->fb.flags		= FBINFO_FLAG_DEFAULT;
	fb->fb.pseudo_palette	= fb->cmap;

	strncpy(fb->fb.fix.id, bcm2708_name, sizeof(fb->fb.fix.id));
	fb->fb.fix.type		= FB_TYPE_PACKED_PIXELS;
	fb->fb.fix.type_aux	= 0;
	fb->fb.fix.xpanstep	= 0;
	fb->fb.fix.ypanstep	= 0;
	fb->fb.fix.ywrapstep	= 0;
	fb->fb.fix.accel	= FB_ACCEL_NONE;

	fb->fb.var.xres		= 800;
	fb->fb.var.yres		= 480;
	fb->fb.var.xres_virtual	= 800;
	fb->fb.var.yres_virtual	= 480 * 2; /* Double buffer */
	fb->fb.var.bits_per_pixel = 16;
	fb->fb.var.vmode	= FB_VMODE_NONINTERLACED;
	fb->fb.var.activate	= FB_ACTIVATE_NOW;
	fb->fb.var.nonstd	= 0;
	fb->fb.var.height	= 800;
	fb->fb.var.width	= 480;
	fb->fb.var.accel_flags	= 0;

	fb->fb.monspecs.hfmin	= 0;
	fb->fb.monspecs.hfmax   = 100000;
	fb->fb.monspecs.vfmin	= 0;
	fb->fb.monspecs.vfmax	= 400;
	fb->fb.monspecs.dclkmin = 1000000;
	fb->fb.monspecs.dclkmax	= 100000000;

	bcm2708_fb_set_bitfields(fb, &fb->fb.var);

    framesize = 800 * 480 * 2 * 2; /* 2 RGB565 buffers of 800x480 */
	fb->fb.screen_base = dma_alloc_writecombine(&fb->dev->dev, framesize,
						    &dma, GFP_KERNEL);
	if (!fb->fb.screen_base) {
		printk(KERN_ERR "BCM2708B0: unable to map framebuffer\n");
		return -ENOMEM;
	}

	fb->fb.fix.smem_start	= dma;
	fb->fb.fix.smem_len	= framesize;

	/*
	 * Allocate colourmap.
	 */

	fb_set_var(&fb->fb, &fb->fb.var);

        printk(KERN_INFO "BCM2708FB: registering framebuffer\n");

	ret = register_framebuffer(&fb->fb);
	if (ret == 0)
		goto out;

	printk(KERN_ERR "BCM2708FB: cannot register framebuffer (%d)\n", ret);

	iounmap(fb->regs);
 out:
	return ret;
}

static int bcm2708_fb_probe(struct platform_device *dev)
{
	struct bcm2708_fb *fb;
	int ret;

	fb = kmalloc(sizeof(struct bcm2708_fb), GFP_KERNEL);
	if (!fb) {
                dev_err(&dev->dev, "could not allocate new bcm2708_fb struct\n");
		ret = -ENOMEM;
		goto free_region;
	}
	memset(fb, 0, sizeof(struct bcm2708_fb));

	fb->dev = dev;

	ret = bcm2708_fb_register(fb);
	if (ret == 0) {
		platform_set_drvdata(dev, fb);
		goto out;
	}

	kfree(fb);
 free_region:
	dev_err(&dev->dev, "probe failed, err %d\n", ret);
 out:
	return ret;
}

static int bcm2708_fb_remove(struct platform_device *dev)
{
	struct bcm2708_fb *fb = platform_get_drvdata(dev);
        
	platform_set_drvdata(dev, NULL);

	unregister_framebuffer(&fb->fb);
	iounmap(fb->regs);

	kfree(fb);

	return 0;
}

static struct platform_driver bcm2708_fb_driver = {
	.probe		= bcm2708_fb_probe,
	.remove		= bcm2708_fb_remove,
	.driver		= {
		.name	= DRIVER_NAME,
		.owner  = THIS_MODULE,
	},
};

static int __init bcm2708_fb_init(void)
{
	return platform_driver_register(&bcm2708_fb_driver);
}

module_init(bcm2708_fb_init);

static void __exit bcm2708_fb_exit(void)
{
	platform_driver_unregister(&bcm2708_fb_driver);
}

module_exit(bcm2708_fb_exit);

MODULE_DESCRIPTION("BCM2708 framebuffer driver");
MODULE_LICENSE("GPL");
