// SPDX-License-Identifier: GPL-2.0

/* Early simple framebuffer driver
 * pass earlycon=simplefb,framebuffer_base,width,height in cmdline
 * assumes 32bpp
*/

#include <linux/console.h>
#include <linux/efi.h>
#include <linux/font.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/serial_core.h>
#include <linux/screen_info.h>

#include <asm/early_ioremap.h>

static const struct console *earlycon_console __initdata;
static const struct font_desc *font;
static u32 simplefb_x, simplefb_y;
static u64 fb_base;
static void *simplefb_fb;

struct screen_info screen_info;

static int __init simplefb_earlycon_remap_fb(void)
{
	/* bail if there is no bootconsole or it has been disabled already */
	if (!earlycon_console || !(earlycon_console->flags & CON_ENABLED))
		return 0;

	simplefb_fb = memremap(fb_base, screen_info.lfb_size, MEMREMAP_WB);

	return simplefb_fb ? 0 : -ENOMEM;
}
early_initcall(simplefb_earlycon_remap_fb);

static int __init simplefb_earlycon_unmap_fb(void)
{
	/* unmap the bootconsole fb unless keep_bootcon has left it enabled */
	if (simplefb_fb && !(earlycon_console->flags & CON_ENABLED))
		memunmap(simplefb_fb);
	return 0;
}
late_initcall(simplefb_earlycon_unmap_fb);

static __ref void *simplefb_earlycon_map(unsigned long start, unsigned long len)
{
	pgprot_t fb_prot;

	if (simplefb_fb)
		return simplefb_fb + start;

	fb_prot = PAGE_KERNEL;
	return early_memremap_prot(fb_base + start, len, pgprot_val(fb_prot));
}

static __ref void simplefb_earlycon_unmap(void *addr, unsigned long len)
{
	if (simplefb_fb)
		return;

	early_memunmap(addr, len);
}

static void simplefb_earlycon_clear_scanline(unsigned int y)
{
	unsigned long *dst;
	u16 len;

	len = screen_info.lfb_linelength;
	dst = simplefb_earlycon_map(y*len, len);
	if (!dst)
		return;

	memset(dst, 0, len);
	simplefb_earlycon_unmap(dst, len);
}

static void simplefb_earlycon_scroll_up(void)
{
	unsigned long *dst, *src;
	u16 len;
	u32 i, height;

	len = screen_info.lfb_linelength;
	height = screen_info.lfb_height;

	for (i = 0; i < height - font->height; i++) {
		dst = simplefb_earlycon_map(i*len, len);
		if (!dst)
			return;

		src = simplefb_earlycon_map((i + font->height) * len, len);
		if (!src) {
			simplefb_earlycon_unmap(dst, len);
			return;
		}

		memmove(dst, src, len);

		simplefb_earlycon_unmap(src, len);
		simplefb_earlycon_unmap(dst, len);
	}
}

static void simplefb_earlycon_write_char(u32 *dst, unsigned char c, unsigned int h)
{
	const u32 color_black = 0x00000000;
	const u32 color_white = 0x00ffffff; // a8r8g8b8
	const u8 *src;
	int m, n, bytes;
	u8 x;

	bytes = BITS_TO_BYTES(font->width);
	src = font->data + c * font->height * bytes + h * bytes;

	for (m = 0; m < font->width; m++) {
		n = m % 8;
		x = *(src + m / 8);
		if ((x >> (7 - n)) & 1)
			*dst = color_white;
		else
			*dst = color_black;
		dst++;
	}
}

static void
simplefb_earlycon_write(struct console *con, const char *str, unsigned int num)
{
	struct screen_info *si;
	unsigned int len;
	const char *s;
	void *dst;

	si = &screen_info;
	len = si->lfb_linelength;

	while (num) {
		unsigned int linemax;
		unsigned int h, count = 0;

		for (s = str; *s && *s != '\n'; s++) {
			if (count == num)
				break;
			count++;
		}

		linemax = (si->lfb_width - simplefb_x) / font->width;
		if (count > linemax)
			count = linemax;

		for (h = 0; h < font->height; h++) {
			unsigned int n, x;

			dst = simplefb_earlycon_map((simplefb_y + h) * len, len);
			if (!dst)
				return;

			s = str;
			n = count;
			x = simplefb_x;

			while (n-- > 0) {
				simplefb_earlycon_write_char(dst + x*4, *s, h);
				x += font->width;
				s++;
			}

			simplefb_earlycon_unmap(dst, len);
		}

		num -= count;
		simplefb_x += count * font->width;
		str += count;

		if (num > 0 && *s == '\n') {
			simplefb_x = 0;
			simplefb_y += font->height;
			str++;
			num--;
		}

		if (simplefb_x + font->width > si->lfb_width) {
			simplefb_x = 0;
			simplefb_y += font->height;
		}

		if (simplefb_y + font->height > si->lfb_height) {
			u32 i;

			simplefb_y -= font->height;
			simplefb_earlycon_scroll_up();

			for (i = 0; i < font->height; i++)
				simplefb_earlycon_clear_scanline(simplefb_y + i);
		}
	}
}

static int __init simplefb_earlycon_setup(struct earlycon_device *device,
				     const char *opt)
{
	struct screen_info *si;
	struct uart_port *port = &device->port;
	u32 xres, yres;
	u32 i;
	int ret;

	si = &screen_info;

	if (!port->mapbase || !*device->options)
		return -ENODEV;

	ret = sscanf(device->options, "%u,%u", &xres, &yres);
	if (ret != 2)
		return -ENODEV;

    si->lfb_linelength = xres * 4;
    si->lfb_width = xres;
    si->lfb_height = yres;
    si->lfb_depth = 32;
    si->lfb_size = si->lfb_width * si->lfb_height * (si->lfb_depth / 8);

	fb_base = port->mapbase;
	xres = si->lfb_width;
	yres = si->lfb_height;

	font = get_default_font(xres, yres, -1, -1);
	if (!font)
		return -ENODEV;

	simplefb_y = rounddown(yres, font->height) - font->height;
	for (i = 0; i < (yres - simplefb_y) / font->height; i++)
		simplefb_earlycon_scroll_up();

	device->con->write = simplefb_earlycon_write;
	earlycon_console = device->con;
	return 0;
}
EARLYCON_DECLARE(simplefb, simplefb_earlycon_setup);
