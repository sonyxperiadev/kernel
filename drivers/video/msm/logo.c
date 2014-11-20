/* drivers/video/msm/logo.c
 *
 * Show Logo in RLE 565 format
 *
 * Copyright (C) 2008 Google Incorporated
 * Copyright (C) 2013 Sony Mobile Communications AB
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fb.h>
#include <linux/vt_kern.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>

#include <linux/irq.h>
#include <asm/system.h>

#define fb_width(fb)	((fb)->var.xres)
#define fb_linewidth(fb) \
	((fb)->fix.line_length / (fb_depth(fb) == 2 ? 2 : 4))
#define fb_height(fb)	((fb)->var.yres)
#define fb_depth(fb)	((fb)->var.bits_per_pixel >> 3)
#define fb_size(fb)	(fb_width(fb) * fb_height(fb) * fb_depth(fb))
#define INIT_IMAGE_FILE "/logo.rle"

static bool display_on_in_boot;

static int __init continous_splash_setup(char *str)
{
	if (!str)
		return 0;
	if (!strncmp(str, "on", 2))
		display_on_in_boot = true;
	return 0;
}
__setup("display_status=", continous_splash_setup);

static void memset16(void *_ptr, unsigned short val, unsigned count)
{
	unsigned short *ptr = _ptr;
	count >>= 1;
	while (count--)
		*ptr++ = val;
}

static void memset32(void *_ptr, unsigned int val, unsigned count)
{
	unsigned int *ptr = _ptr;
	count >>= 2;
	while (count--)
		*ptr++ = val;
}

/* 565RLE image format: [count(2 bytes), rle(2 bytes)] */
int load_565rle_image(char *filename)
{
	struct fb_info *info;
	int fd, err = 0;
	unsigned count, max, width, stride, line_pos = 0;
	unsigned short *data, *ptr;
	unsigned char *bits;

	info = registered_fb[0];
	if (!info) {
		printk(KERN_WARNING "%s: Can not access framebuffer\n",
			__func__);
		return -ENODEV;
	}

	fd = sys_open(filename, O_RDONLY, 0);
	if (fd < 0) {
		printk(KERN_WARNING "%s: Can not open %s\n",
			__func__, filename);
		return -ENOENT;
	}
	count = sys_lseek(fd, (off_t)0, 2);
	if (count <= 0) {
		err = -EIO;
		goto err_logo_close_file;
	}
	sys_lseek(fd, (off_t)0, 0);
	data = kmalloc(count, GFP_KERNEL);
	if (!data) {
		printk(KERN_WARNING "%s: Can not alloc data\n", __func__);
		err = -ENOMEM;
		goto err_logo_close_file;
	}
	if (sys_read(fd, (char *)data, count) != count) {
		err = -EIO;
		goto err_logo_free_data;
	}
	width = fb_width(info);
	stride = fb_linewidth(info);
	max = width * fb_height(info);
	ptr = data;
	bits = (unsigned char *)(info->screen_base);

	while (count > 3) {
		int n = ptr[0];

		if (n > max)
			break;
		max -= n;
		while (n > 0) {
			unsigned int j =
				(line_pos + n > width ? width-line_pos : n);

			if (fb_depth(info) == 2)
				memset16(bits, ptr[1], j << 1);
			else {
				unsigned int widepixel = ptr[1];
				/*
				 * Format is RGBA, but fb is big
				 * endian so we should make widepixel
				 * as ABGR.
				 */
				widepixel =
					/* red :   f800 -> 000000f8 */
					(widepixel & 0xf800) >> 8 |
					/* green : 07e0 -> 0000fc00 */
					(widepixel & 0x07e0) << 5 |
					/* blue :  001f -> 00f80000 */
					(widepixel & 0x001f) << 19;
				memset32(bits, widepixel, j << 2);
			}
			bits += j * fb_depth(info);
			line_pos += j;
			n -= j;
			if (line_pos == width) {
				bits += (stride-width) * fb_depth(info);
				line_pos = 0;
			}
		}
		ptr += 2;
		count -= 4;
	}

err_logo_free_data:
	kfree(data);
err_logo_close_file:
	sys_close(fd);

	return err;
}

static void __init draw_logo(void)
{
	struct fb_info *fb_info;

	fb_info = registered_fb[0];
	if (fb_info && fb_info->fbops->fb_open) {
		printk(KERN_INFO "Drawing logo.\n");
		fb_info->fbops->fb_open(fb_info, 0);
		fb_info->fbops->fb_pan_display(&fb_info->var, fb_info);
	}
}

int __init logo_init(void)
{
	if (display_on_in_boot)
		printk(KERN_INFO "Skip drawing logo. Already drawn in boot.\n");
	else if (!load_565rle_image(INIT_IMAGE_FILE))
		draw_logo();

	return 0;
}

module_init(logo_init);
