#ifndef _fb_fps_H_ 
#define _fb_fps_H_	1

#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/proc_fs.h>

#define FPS_CALC_INTERVAL	5 /* update fps after how many frames */
#define FPS_STR_LEN		20
#define MAX_CELLSIZE	20  /* Max size of cell per character */

struct fb_fps_info {
	struct fb_info *fb;
	struct console_font font;		/* Current VC font set */
	unsigned char pixmap[MAX_CELLSIZE * FPS_STR_LEN]; /* max cellsize 20 */
	struct fb_image image;
	int frame_count;
	unsigned long  interval_start_time;
	unsigned int frame_calc_interval;
	struct proc_dir_entry *proc_entry;
};

struct fb_fps_info * fb_fps_register(struct fb_info *info);

void fb_fps_unregister(struct fb_fps_info *fps_info);

void fb_fps_display(struct fb_fps_info *fps_info, void *buffer, int x, int y, int disp_now);
#endif /* _fb_fps_H_ */

