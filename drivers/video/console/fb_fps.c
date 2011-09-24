#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/delay.h>	/* MSch: for IRQ probe */
#include <linux/console.h>
#include <linux/string.h>
#include <linux/kd.h>
#include <linux/slab.h>
#include <linux/fb.h>
#include <linux/vt_kern.h>
#include <linux/selection.h>
#include <linux/font.h>
#include <linux/smp.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include <linux/fb_fps.h>

#include "fbcon.h"


static void update_str_attr(u8 *dst, u8 *src, int attribute,
			       struct console_font *font)
{
	int i, offset = (font->height < 10) ? 1 : 2;
	int width = DIV_ROUND_UP(font->width, 8);
	unsigned int cellsize = font->height * width;
	u8 c;

	offset = cellsize - (offset * width);
	for (i = 0; i < cellsize; i++) {
		c = src[i];
		if (attribute & FBCON_ATTRIBUTE_UNDERLINE && i >= offset)
			c = 0xff;
		if (attribute & FBCON_ATTRIBUTE_BOLD)
			c |= c >> 1;
		if (attribute & FBCON_ATTRIBUTE_REVERSE)
			c = ~c;
		dst[i] = c;
	}
}

static inline void bit_putstr_aligned(struct console_font *font, struct fb_info *info,
				     const u16 *s, u32 attr, u32 cnt,
				     u32 d_pitch, u32 s_pitch, u32 cellsize,
				     struct fb_image *image, u8 *buf, u8 *dst)
{
	/*
	u16 charmask = vc->vc_hi_font_mask ? 0x1ff : 0xff;
	*/
	u16 charmask = 0xff; /* we dont support hi_font */
	u32 idx = font->width >> 3;
	u8 *src;

	while (cnt--) {
		src = font->data + (scr_readw(s++)&
					  charmask)*cellsize;

		if (attr) {
			update_str_attr(buf, src, attr, font);
			src = buf;
		}

		if (likely(idx == 1))
			__fb_pad_aligned_buffer(dst, d_pitch, src, idx,
						image->height);
		else
			fb_pad_aligned_buffer(dst, d_pitch, src, idx,
					      image->height);

		dst += s_pitch;
	}

}

static inline void bit_putstr_unaligned(struct console_font *font,
				       struct fb_info *info, const u16 *s,
				       u32 attr, u32 cnt, u32 d_pitch,
				       u32 s_pitch, u32 cellsize,
				       struct fb_image *image, u8 *buf,
				       u8 *dst)
{
	/*
	u16 charmask = vc->vc_hi_font_mask ? 0x1ff : 0xff;
	*/
	u16 charmask = 0xff; /* we dont support hi_font */
	u32 shift_low = 0, mod = font->width % 8;
	u32 shift_high = 8;
	u32 idx = font->width >> 3;
	u8 *src;

	while (cnt--) {
		src = font->data + (scr_readw(s++)&
					  charmask)*cellsize;

		if (attr) {
			update_str_attr(buf, src, attr, font);
			src = buf;
		}

		fb_pad_unaligned_buffer(dst, d_pitch, src, idx,
					image->height, shift_high,
					shift_low, mod);
		shift_low += mod;
		dst += (shift_low >= 8) ? s_pitch : s_pitch - 1;
		shift_low &= 7;
		shift_high = 8 - shift_low;
	}
}


void bit_putstr(struct console_font *font, struct fb_info *info, u8 *pixmap,
		u32 pixsize, const unsigned short *s, int count, int yy, int xx,
		      int fg, int bg,struct fb_image *image)
{
	u32 width = DIV_ROUND_UP(font->width, 8);
	u32 cellsize = width * font->height;
	u32 maxcnt = pixsize/cellsize;
	u32 scan_align = 0;
	u32 mod = font->width % 8, cnt, pitch;
	u32 attribute = get_attribute(info, scr_readw(s));
	u8 *dst, *buf = NULL;

	image->fg_color = fg;
	image->bg_color = bg;
	image->dx = xx * font->width;
	image->dy = yy * font->height;
	image->height = font->height;
	image->depth = 1;

	if (attribute) {
		buf = kmalloc(cellsize, GFP_KERNEL);
		if (!buf)
			return;
	}

	if (count > maxcnt)
		cnt = maxcnt;
	else
		cnt = count;

	image->width = font->width * cnt;
	pitch = DIV_ROUND_UP(image->width, 8) + scan_align;
	pitch &= ~scan_align;
	/*
	   size = pitch * image->height + buf_align;
	   size &= ~buf_align;
	   dst = fb_get_buffer_offset(info, &info->pixmap, size);
	   */
	image->data = dst = pixmap ;

	if (!mod)
		bit_putstr_aligned(font, info, s, attribute, cnt, pitch,
				width, cellsize, image, buf, dst);
	else
		bit_putstr_unaligned(font, info, s, attribute, cnt,
				pitch, width, cellsize, image,
				buf, dst);
/*
	image->dx += cnt * font->width;
	count -= cnt;
	s += cnt;
*/

	/* buf is always NULL except when in monochrome mode, so in this case
	   it's a gain to check buf against NULL even though kfree() handles
	   NULL pointers just fine */
	if (unlikely(buf))
		kfree(buf);

}

static void fb_fps_str_to_img( struct fb_fps_info *fps_info, const char *src, unsigned count,
		int x, int y)
{
	unsigned short vc_str[FPS_STR_LEN];
	unsigned char vc_attr = 0x7;
	int i;
	
	for (i=0; i < count; i++, src++)
		vc_str[i] = ((vc_attr << 8) + *src);

	bit_putstr(&(fps_info->font), fps_info->fb, fps_info->pixmap,
			FPS_STR_LEN*MAX_CELLSIZE, vc_str, count, y, x, 7, 0, &(fps_info->image));

}

static int
proc_read_fb_fps(char *buf, char **start, off_t offset,
		       int len, int *eof, void *data)
{
	int *val = (int *)data;
	len = sprintf(buf, "FPS will be recalulated after every %d frames", *val);
	return len;
}

static int
proc_write_fb_fps(struct file *file, const char __user *buffer,
			unsigned long count, void *data)
{
	int len;
	int *val = (int *)data;
	char value[20];
	int no_of_frames;

	if (count > 19)
		len = 19;
	else
		len = count;

	if (copy_from_user(value, buffer, len))
		return -EFAULT;

	 value[len] = '\0';

	 no_of_frames = simple_strtol(value, NULL, 10);

	 if (no_of_frames > 0) {
		 printk(KERN_INFO "%s: FPS will be recalulated after every %d frames\n", __func__, no_of_frames);
		 *val = no_of_frames;
	 }
	 else
		 printk(KERN_ERR "%s:wrong value passed. value cannot be %s\n", __func__, value);

	 return len;
}

struct fb_fps_info * fb_fps_register(struct fb_info *info)
{
	struct fb_fps_info *fps_info;
	const struct font_desc *f;
	char default_str[] = "0.0";
	
	if (NULL == info->fbops->fb_imageblit) {
		printk(KERN_ERR"%s:fps display needs image blitter\n", __func__ );
		goto err;
	}
	
	fps_info = kzalloc(sizeof(struct fb_fps_info), GFP_KERNEL);
	if (!fps_info) {
		printk(KERN_ERR "%s: could not allocate new fps_info struct\n", __func__);
		goto err;
	}
	
	f = get_default_font(info->var.xres, info->var.yres,
				     info->pixmap.blit_x, info->pixmap.blit_y);

	fps_info->font.width = f->width;
	fps_info->font.height = f->height;
	fps_info->font.data = (unsigned char *)f->data;
	fps_info->font.charcount = 256; /*FIXME? more fonts */
	fps_info->fb = info;
	fps_info->interval_start_time = jiffies;
	fps_info->frame_count = 0;
	fps_info->frame_calc_interval = FPS_CALC_INTERVAL;

	fps_info->proc_entry = create_proc_entry("fb_fps", 0666, NULL);

	if (NULL == fps_info->proc_entry)
		printk(KERN_ERR "%s: could not create proc entry."
				"FPS will be calculated after every %d frames\n", __func__, FPS_CALC_INTERVAL);
	else {
		/* well we could take a lock to protect frame_calc_interval.Since its
		 * used after every 5 frames we can avoid locking and max the
		 * calculations can go wrong for 5 frames */
		fps_info->proc_entry->data = &(fps_info->frame_calc_interval);
		fps_info->proc_entry->read_proc = proc_read_fb_fps;
		fps_info->proc_entry->write_proc = proc_write_fb_fps;
	}

	fb_fps_str_to_img(fps_info, default_str, strlen(default_str), 0, 0); 
	return fps_info;
err:
	return NULL;
}
EXPORT_SYMBOL(fb_fps_register);

void fb_fps_unregister(struct fb_fps_info *fps_info)
{
	if( NULL == fps_info)
		goto out;
	remove_proc_entry("fb_fps", NULL);
	kfree(fps_info);
out:
	return;
}
EXPORT_SYMBOL(fb_fps_unregister);



void fb_fps_display(struct fb_fps_info *fps_info, void *dst, int x, int y, int disp_now)
{
	unsigned long curr_time = jiffies;
	char print_str[FPS_STR_LEN];
	unsigned int tms;
	unsigned int fps_quotient;
	unsigned int fps_decimal;
	void *screen_base;

	if( NULL == fps_info )
		goto out;
	
	if( NULL == dst ) {
		printk(KERN_ERR"%s need destination\n", __func__);
		goto out;
	}

	
	if (fps_info->frame_count >= fps_info->frame_calc_interval || disp_now) {
		
		tms = jiffies_to_msecs(curr_time - fps_info->interval_start_time);
		if (tms != 0) {
			fps_quotient = (fps_info->frame_calc_interval * 1000)/tms;
			fps_decimal  = ( ( (fps_info->frame_calc_interval * 1000)%tms ) * 100 ) /tms;
			snprintf(print_str, FPS_STR_LEN, "%d.%d", fps_quotient, fps_decimal);
		} else {
			snprintf(print_str, FPS_STR_LEN, "*.*");
			printk(KERN_ERR "This should not happen. start time=%lu jiffies end time=%lu jiffies tms=%d",
					fps_info->interval_start_time, curr_time, tms);
		}

		fb_fps_str_to_img(fps_info, print_str, strlen(print_str), x, y);
		fps_info->frame_count = 0;
		fps_info->interval_start_time = curr_time;
	}

	/* HACK */
	screen_base = fps_info->fb->screen_base;
	fps_info->fb->screen_base = dst;
	fps_info->fb->fbops->fb_imageblit(fps_info->fb, &(fps_info->image));
	fps_info->fb->screen_base = screen_base;
	
	if (!disp_now)
		fps_info->frame_count++;
out:
	return;
}
EXPORT_SYMBOL(fb_fps_display);

static int __init fb_fps_init(void)
{
	printk(KERN_INFO"Broadcom framebuffer fps library\n");
	return 0;
}
module_init(fb_fps_init);


static void __exit fb_fps_exit(void)
{
	printk(KERN_INFO"Broadcom framebuffer fps library removed\n");
}
module_exit(fb_fps_exit);
