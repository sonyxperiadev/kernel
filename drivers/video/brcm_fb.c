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

#include <mach/hardware.h>
#include <mach/io.h>
#include <mach/memory.h>
#include <mach/platform.h>

#ifdef CONFIG_ANDROID_POWER
#include <linux/android_power.h>
#endif

#ifdef USE_FB_IRQ
#error "DO NOT USE FB IRQ YET"
#endif

#define brcmfb_debug(fmt, arg...)	\
	printk(KERN_DEBUG"%s:%d " fmt, __func__, __LINE__, ##arg)

#define brcmfb_info(fmt, arg...)	\
	printk(KERN_INFO"%s:%d " fmt, __func__, __LINE__, ##arg)

#define brcmfb_error(fmt, arg...)	\
	printk(KERN_ERR"%s:%d " fmt, __func__, __LINE__, ##arg)

#define brcmfb_warning(fmt, arg...)	\
	printk(KERN_WARNING"%s:%d " fmt, __func__, __LINE__, ##arg)

#define brcmfb_alert(fmt, arg...)	\
	printk(KERN_ALERT"%s:%d " fmt, __func__, __LINE__, ##arg)

#ifdef IPC_BLOCK_IS_READY
#include <ipc_block.h> /* This name may change ..*/
#else
/* Temporary definitions for framebuffer registers */

#define IPC_MEM_BASE			(0x07C00000)
#define	IPC_MEM_SIZE			(SZ_16K)
#define FB_FOURCC			'FBUF'
#define FB_IRQ				(1)
#define IPC_VC_INTERRUPT_OFFSET		(0x804)
#define FB_MEM_BASE			(IPC_MEM_BASE + (SZ_16K * 2))
#define DOORBELL_NUM			(2)

#define FB_CONTROL			(0x00)
	#define	CTRL_ENABLE_SHIFT	(0)
	#define CTRL_ENABLE_MASK	(0x00000001)
	#define CTRL_DISP_EN_SHIFT	(1)
	#define CTRL_DISP_EN_MASK	(0x00000002)
	#define CTRL_BUFF_IDX_SHIFT	(8)
	#define CTRL_BUFF_IDX_MASK	(0x00000f00)
#define FB_STATUS			(0x04)	/* read only */
	#define STAT_EN_SHIFT		(0)
	#define STAT_EN_MASK		(0x00000001)
	#define STAT_DISP_SHIFT		(1)
	#define STAT_DISP_MASK		(0x00000002)
	#define STAT_BUFF_IDX_SHIFT	(4)
	#define STAT_BUFF_IDX_MASK	(0x000000f0)
	#define	STAT_UPDATE_COUNT_SHIFT	(8)	
	#define STAT_UPDATE_COUNT_MASK	(0xffffff00)
#define FB_WIDTH			(0x20)	/* read only */
#define FB_HEIGHT			(0x24)	/* read only */
#define FB_SET_FORMAT			(0x28)
	#define FB_FORMAT_RGB565	(0)
#define FB_SET_NUM_BUFFERS		(0x2C)
#define FB_SET_DISPLAY			(0x40)

#define FB_GET_BUFFER_PHYS		(0x60)	/* read only */
#define FB_TARGET_LAYER			(0x44)
#define FB_TARGET_X			(0x48)
#define FB_TARGET_Y			(0x4C)
#define FB_TARGET_WIDTH			(0x50)
#define FB_TARGET_HEIGHT		(0x54)

#define IRQ_FRAMEBUFFER			(0)

//extern uint32_t ipc_block_register(uint32_t fourcc, uint32_t irq,
//				irq_handler_t handler, void *data);
#endif

#define DEBUG_FRAMEBUFFER

struct brcm_fb {
	struct resource *regs_res;
	struct resource *fbmem_res;
	phys_addr_t phys_regbase;
	phys_addr_t phys_fbbase;
	size_t regmap_size;
	void __iomem *reg_base;
	void __iomem *fbmem;
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
#ifndef IPC_BLOCK_IS_READY
	void __iomem *ipc_base;
#endif
};

#ifdef DEBUG_FRAMEBUFFER
struct proc_dir_entry *fb_proc;
static unsigned char paint = 0x0;
static uint32_t buff_num = 0;
struct brcm_fb *global_hack;
static int brcm_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info);

static int fb_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	struct fb_info *info = &global_hack->fb;
	uint32_t offset;
	void *fbdata;
	uint32_t framesize;

	framesize = info->var.xres * info->var.yres * 2;

	brcmfb_info("Updating framebuffer with 0x%2x%2x\n",paint, paint);
	info->var.yoffset = buff_num * info->var.yres;
	buff_num = buff_num ? 0 : 1;
	offset = info->var.xres * 2 * info->var.yoffset;
	fbdata = (void *)(info->screen_base + offset);
	memset(fbdata, paint, framesize); 
	paint = ~paint;
	brcmfb_info("FB Update starting ..!\n");
	brcm_fb_pan_display(&info->var, info);
	brcmfb_info("FB Update done !\n");

	return count;
}
#endif

#ifndef IPC_BLOCK_IS_READY
static void ipc_initialise(struct brcm_fb *fb)
{
	fb->ipc_base = ioremap(IPC_MEM_BASE, IPC_MEM_SIZE);
	if (fb->ipc_base == NULL) {
		brcmfb_error("Failed to map ipc registers\n");
		/* Hack : fail ..*/
		*(uint32_t *)0 = 1;
	}

	brcmfb_info("Successfull, base is at phys(0x%08x), virt(0x%08x)...\n", IPC_MEM_BASE,(uint32_t)fb->ipc_base);
}

static void ipc_ring_doorbell(struct brcm_fb *fb, uint32_t irq_num)
{
	uint32_t regval;
	volatile unsigned long *doorbell;
	/* supposed to take hw semaphore here ... */

	regval = readl(fb->ipc_base + IPC_VC_INTERRUPT_OFFSET);
	regval |= (1 << irq_num);
	brcmfb_debug("Writing (0x%08x) at (0x%08x)\n", regval, (fb->ipc_base + IPC_VC_INTERRUPT_OFFSET));
	writel(regval, fb->ipc_base + IPC_VC_INTERRUPT_OFFSET);

	/* supposed to leave hw semaphore here .. */

	/* Now fire the interrupt to VC .. */
	doorbell = IO_ADDRESS(ARM_0_BELL2); /* bell = 2, owner = 0 */
	brcmfb_debug("Doorbell address = phys 0x%08x, 0x%08x\n", ARM_0_BELL2,doorbell);
	*doorbell = 1;
}
#endif

static inline u32 convert_bitfield(int val, struct fb_bitfield *bf)
{
	unsigned int mask = (1 << bf->length) - 1;

	return (val >> (16 - bf->length) & mask) << bf->offset;
}

static int
brcm_fb_setcolreg(unsigned int regno, unsigned int red, unsigned int green,
		 unsigned int blue, unsigned int transp, struct fb_info *info)
{
	struct brcm_fb *fb = container_of(info, struct brcm_fb, fb);

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

static int brcm_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	if((var->rotate & 1) != (info->var.rotate & 1)) {
		if((var->xres != info->var.yres) ||
		   (var->yres != info->var.xres) ||
		   (var->xres_virtual != info->var.yres) ||
		   (var->yres_virtual > info->var.xres * 2) ||
		   (var->yres_virtual < info->var.xres )) {
			brcmfb_error("fb_check_var_failed\n");
			return -EINVAL;
		}
	} else {
		if((var->xres != info->var.xres) ||
		   (var->yres != info->var.yres) ||
		   (var->xres_virtual != info->var.xres) ||
		   (var->yres_virtual > info->var.yres * 2) ||
		   (var->yres_virtual < info->var.yres )) {
			brcmfb_error("fb_check_var_failed\n");
			return -EINVAL;
		}
	}

	if((var->xoffset != info->var.xoffset) ||
	   (var->bits_per_pixel != info->var.bits_per_pixel) ||
	   (var->grayscale != info->var.grayscale)) {
		brcmfb_error("fb_check_var_failed\n");
		return -EINVAL;
	}

	if ((var->yoffset != 0) &&
		(var->yoffset != info->var.yres)) {
		brcmfb_error("fb_check_var failed\n");
		brcmfb_alert("BRCM fb does not support partial FB updates\n");
		return -EINVAL;
	}

	return 0;
}

static int brcm_fb_set_par(struct fb_info *info)
{
	struct brcm_fb *fb = container_of(info, struct brcm_fb, fb);
	if(fb->rotation != fb->fb.var.rotate) {
		brcmfb_warning("Rotation is not supported yet !\n");
		return -EINVAL;
	}

	return 0;
}

static int brcm_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	unsigned long irq_flags;
	int ret;
	struct brcm_fb *fb = container_of(info, struct brcm_fb, fb);
	uint32_t buff_idx, control, status;
	uint32_t vc_update_count;
	long timeout;
	
	/* We are here only if yoffset = 0/yres,
	 * so if yoffset = 0, update first buffer or update second
	 */
	buff_idx = var->yoffset ? 1 : 0;

	spin_lock_irqsave(&fb->lock, irq_flags);
	control = readl(fb->reg_base + FB_CONTROL);
	control &= ~(CTRL_BUFF_IDX_MASK);
	control |= ((buff_idx << CTRL_BUFF_IDX_SHIFT)); 
	/* If this is the first update, enable framebuffer display first */
	writel(control, fb->reg_base + FB_CONTROL);
	fb->base_update_count++;

	/* ring the doorbell */
	ipc_ring_doorbell(fb, FB_IRQ);
	spin_unlock_irqrestore(&fb->lock, irq_flags);

#ifdef USE_FB_IRQ
	timeout = HZ*3
	ret = down_timeout(&fb->wait_for_irq, timeout); /* 3 sec timeout */
	if (ret != 0) {
		brcmfb_warning("BRCM fb update timed out, no irq received\n");
		return ret;
	}
#else
	/* poll to read status and timeout after 3 secs */
	timeout = jiffies + HZ*3;
	do {
		status = readl(fb->reg_base + FB_STATUS);
		brcmfb_info("status = (0x%08x)\n", status);
		vc_update_count = status >> STAT_UPDATE_COUNT_SHIFT;
#if 0
		if (jiffies > timeout) {
			brcmfb_error("Framebuffer update timed out\n");
			ret = -EAGAIN;
			break;
		}
#endif
		brcmfb_info("Now sleeping cause host_update = %d, vc_update = %d..\n",
				fb->base_update_count & 0x00ffffff, vc_update_count);
	} while (((fb->base_update_count & 0x00ffffff) != vc_update_count));

#if 0
	/* Extra check */
	brcmfb_info("Extra check for buffer index (%d) against (%d)\n",
			buff_idx, (status & STAT_BUFF_IDX_SHIFT) >> STAT_BUFF_IDX_SHIFT);
	if (buff_idx != ((status & STAT_BUFF_IDX_MASK) >> STAT_BUFF_IDX_SHIFT)) {
		brcmfb_error("update buffer index/display do not match\n");
		ret = -EAGAIN;
	}
#endif
#endif

	return ret;
}

#ifdef CONFIG_ANDROID_POWER
static void brcm_fb_early_suspend(android_early_suspend_t *h)
{
	struct brcm_fb *fb = container_of(h, struct brcm_fb, early_suspend);
	brcmfb_info("TODO: BRCM fb early suspend ...\n");
}

static void brcm_fb_late_resume(android_early_suspend_t *h)
{
	struct brcm_fb *fb = container_of(h, struct brcm_fb, early_suspend);
	brcmfb_info("TODO: BRCM fb late resume ...\n");
}
#endif

static struct fb_ops brcm_fb_ops = {
	.owner          = THIS_MODULE,
	.fb_check_var   = brcm_fb_check_var,
	.fb_set_par     = brcm_fb_set_par,
	.fb_setcolreg   = brcm_fb_setcolreg,
	.fb_pan_display = brcm_fb_pan_display,
	.fb_fillrect    = cfb_fillrect,
	.fb_copyarea    = cfb_copyarea,
	.fb_imageblit   = cfb_imageblit,
};

static int enable_fb_device(struct brcm_fb *fb)
{
	uint32_t status, control;
	int ret;
	uint32_t ready_mask = STAT_EN_MASK | STAT_DISP_MASK;
	/* Set width, height, target display, format and num buffers and then
	 * enable framebuffer device
	 */

	writel(2, fb->reg_base + FB_SET_DISPLAY); /* HDMI only for now */
	writel(1280, fb->reg_base + FB_WIDTH); /* 1280x720 */
	writel(720, fb->reg_base + FB_HEIGHT); /* 1280x720 */
	writel(0, fb->reg_base + FB_SET_FORMAT); /*565 only .. */
	writel(2, fb->reg_base + FB_SET_NUM_BUFFERS); /* double buffered */

	/* Write control register bits ...*/
	control = readl(fb->reg_base + FB_CONTROL);
	control |= ((1 << CTRL_ENABLE_SHIFT) | (1 << CTRL_DISP_EN_SHIFT)); 
	writel(control, fb->reg_base + FB_CONTROL);

	/* for VC interrupt .. */
	ipc_ring_doorbell(fb, FB_IRQ);

	/* Wait for Framebuffer to get enabled .. */
#ifdef USE_FB_IRQ
	ret = down_timeout(&fb->wait_for_irq, HZ*3);
	if (ret) {
		brcmfb_warning("Wait for fb interrupt timed out ..\n");
		goto out;
	}
#else
	while(((status = readl(fb->reg_base + FB_STATUS)) & ready_mask) != ready_mask) 
		brcmfb_info("Waiting for fb to come up, status = (0x%08x) ..\n", status);
#endif
	brcmfb_info("Got status = 0x%08x\n",status);

	fb->base_update_count++;
	fb->phys_fbbase = readl(fb->reg_base + FB_GET_BUFFER_PHYS);
	if (!fb->phys_fbbase) {
		brcmfb_error("FB phys memory pointer is NULL\n");
		goto out;
	}

	/* Convert VC phys addr to Kernel phys addrd */
	fb->phys_fbbase = __bus_to_phys(fb->phys_fbbase);

	return 0;

out:
	return ret;

}

static void disable_fb_device(struct brcm_fb *fb)
{
	uint32_t control;

	/* Write control register bits ...*/
	control = readl(fb->reg_base + FB_CONTROL);
	control &= ~CTRL_ENABLE_MASK; 
	writel(control, fb->reg_base + FB_CONTROL);

	ipc_ring_doorbell(fb, FB_IRQ);
}

static int brcm_fb_probe(struct platform_device *pdev)
{
	int ret = -ENXIO;
	struct resource *r;
	struct brcm_fb *fb;
	size_t framesize;
	uint32_t width, height;

	fb = kzalloc(sizeof(struct brcm_fb), GFP_KERNEL);
	if (fb == NULL) {
		brcmfb_error("Unable to allocate framebuffer structure\n");
		ret = -ENOMEM;
		goto err_fb_alloc_failed;
	}

#ifdef DEBUG_FRAMEBUFFER
	global_hack = fb;
#endif
	spin_lock_init(&fb->lock);
	sema_init(&fb->wait_for_irq, 0);
	platform_set_drvdata(pdev, fb);
#ifdef IPC_BLOCK_IS_READY
	// handle = ipc_block_register(fourcc, irq, irq_handler_t, data, &notify_callback);
#else
	ipc_initialise(fb);
#endif
	
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(r == NULL) {
		brcmfb_error("Unable to get framebuffer memory resource\n");
		ret = -ENODEV;
		goto err_no_io_base;
	}

	fb->phys_regbase = r->start;
	fb->regmap_size = resource_size(r);

	fb->irq = platform_get_irq(pdev, 0);
	if(fb->irq < 0) {
		brcmfb_error("Unable to get framebuffer irq resource\n");
		ret = -ENODEV;
		goto err_no_irq;
	}

	/* ioremap the framebuffer register memory 
	 */
	
	brcmfb_info("Requesting and mapping resource (0x%08x)-(0x%08x)\n",
			r->start, r->end);

	fb->regs_res = request_mem_region(r->start,resource_size(r), "brcmfb_regs");
	if (fb->regs_res == NULL) {
		brcmfb_error("Unable to request framebuffer memory region\n");
		ret = -ENOMEM;
		goto err_no_mem_region;

	}

	fb->reg_base = ioremap(fb->phys_regbase, fb->regmap_size);
	if (fb->reg_base == NULL) {
		brcmfb_error("Unable to ioremap fb regs memory\n");
		ret = -ENOMEM;
		goto err_ioremap_failed;
	}

	
	ret = enable_fb_device(fb);
	if (ret) {
		brcmfb_error("Failed to enable FB device\n");
		goto err_enable_fb_failed;
	}
	
	/* Now we should get correct width and height for this display .. */
	width = readl(fb->reg_base + FB_WIDTH);
	height = readl(fb->reg_base + FB_HEIGHT);

	fb->fb.fbops		= &brcm_fb_ops;
	fb->fb.flags		= FBINFO_FLAG_DEFAULT;
	fb->fb.pseudo_palette	= fb->cmap;
	//strncpy(fb->fb.fix.id, clcd_name, sizeof(fb->fb.fix.id));
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
	fb->fb.var.height	= readl(fb->reg_base + FB_HEIGHT);
	fb->fb.var.width	= readl(fb->reg_base + FB_WIDTH);

	fb->fb.var.red.offset = 11;
	fb->fb.var.red.length = 5;
	fb->fb.var.green.offset = 5;
	fb->fb.var.green.length = 6;
	fb->fb.var.blue.offset = 0;
	fb->fb.var.blue.length = 5;

	framesize = width * height * 2 * 2;

	fb->fb.fix.smem_start = fb->phys_fbbase;
	fb->fb.fix.smem_len = framesize;

	brcmfb_debug("Requesting and mapping resource (0x%08x)-(0x%08x)\n",
			r->start, r->end);
	fb->fbmem_res = request_mem_region(fb->phys_fbbase, framesize, "Framebuffer Memory");
	if (fb->fbmem_res == NULL) {
		ret = -ENOMEM;
		brcmfb_error("Unable to get fb memory resource\n");
		goto err_get_fbmem_resource_failed;

	}

	fb->fb.screen_base = ioremap(fb->fbmem_res->start, resource_size(fb->fbmem_res)); 
	if (fb->fb.screen_base == NULL) {
		ret = -ENOMEM;
		brcmfb_error("Unable to ioremap fb memory resource\n");
		goto err_ioremap_fbmem_failed;
	}

	brcmfb_info("Framebuffer starts at phys[0x%08x], and virt[0x%08x]\n",
			fb->phys_fbbase, (uint32_t)fb->fb.screen_base);

	ret = fb_set_var(&fb->fb, &fb->fb.var);
	if (ret) {
		brcmfb_error("fb_set_var failed\n");
		goto err_set_var_failed;
	}

	/* Paint it black (assuming default fb contents are all zero) */
	brcm_fb_pan_display(&fb->fb.var, &fb->fb);

	ret = register_framebuffer(&fb->fb);
	if (ret) {
		brcmfb_error("Framebuffer registration failed\n");
		goto err_fb_register_failed;
	}

	brcmfb_info("BRCM Framebuffer probe successfull\n");

#ifdef CONFIG_ANDROID_POWER
	fb->early_suspend.suspend = brcm_fb_early_suspend;
	fb->early_suspend.resume = brcm_fb_late_resume;
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
	/* todo: unregister ipc block driver */
	iounmap(fb->reg_base);
err_ioremap_failed:
	release_region(fb->regs_res->start, resource_size(fb->regs_res));
err_no_mem_region:
err_no_irq:
err_no_io_base:
	kfree(fb);
err_fb_alloc_failed:
	brcmfb_alert("BRCM Framebuffer probe FAILED !!\n");
	return ret;
}

static int brcm_fb_remove(struct platform_device *pdev)
{
	size_t framesize;
	struct brcm_fb *fb = platform_get_drvdata(pdev);
	
	framesize = fb->fb.var.xres_virtual * fb->fb.var.yres_virtual * 2;

#ifdef CONFIG_ANDROID_POWER
        android_unregister_early_suspend(&fb->early_suspend);
#endif
	unregister_framebuffer(&fb->fb);
	disable_fb_device(fb);
	brcmfb_warning("BRCM FB remove is only partially implemented\n");
	kfree(fb);
	return 0;
}

static struct platform_driver brcm_fb_driver = {
	.probe		= brcm_fb_probe,
	.remove		= brcm_fb_remove,
	.driver = {
		.name = "brcm_fb"
	}
};

static struct resource brcm_fb_resources[] = {
	{
		.start	= FB_MEM_BASE,
		.end	= FB_MEM_BASE + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= IRQ_FRAMEBUFFER,
		.end	= IRQ_FRAMEBUFFER,
		.flags	= IORESOURCE_IRQ,
	}
};

static struct platform_device brcm_fb_device = {
	.name		= "brcm_fb",
	.id		= -1,
	.resource	= brcm_fb_resources,
	.num_resources	= 2,
	.dev		= {
		.coherent_dma_mask = 0xffffffff,
	},
};

static int __init brcm_fb_init(void)
{
	int ret;

	/* FIXME:ssp check if this device exists ?? */

	ret = platform_device_register(&brcm_fb_device);
	if (ret) {
		printk(KERN_ERR"%s : Unable to register BRCM fb deviec\n", __func__);
		goto out;
	}

	ret = platform_driver_register(&brcm_fb_driver);
	if (ret)
		printk(KERN_ERR"%s : Unable to register BRCM battery driver\n", __func__);

#ifdef DEBUG_FRAMEBUFFER
	fb_proc = create_proc_entry("fbtest", 0666, NULL);
	if (fb_proc == NULL) {
		brcmfb_error("Failed to create proc entry\n");
	}

	fb_proc->read_proc = NULL;
	fb_proc->write_proc = fb_proc_write;
	fb_proc->mode = S_IFREG | S_IRUGO;
	fb_proc->uid = 0;
	fb_proc->gid = 0;
#endif

	printk(KERN_INFO"BRCM Framebuffer Init %s !\n", ret ? "FAILED" : "OK");
out:
	return ret;
}

static void __exit brcm_fb_exit(void)
{
#ifdef DEBUG_FRAMEBUFFER
	remove_proc_entry("fbtest", NULL);
#endif
	/* Clean up .. */
	platform_driver_unregister(&brcm_fb_driver);
	platform_device_unregister(&brcm_fb_device);

	printk(KERN_INFO"BRCM Framebuffer exit OK\n");
}

module_init(brcm_fb_init);
module_exit(brcm_fb_exit);
