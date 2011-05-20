#ifndef VCEB_FB_H_
#define VCEB_FB_H_

enum vcebfb_display_type {
	VCEB_DISPLAY_TYPE_3_2 = 0,
	VCEB_DISPLAY_TYPE_4_8 = 1,
};

enum vcebfb_gpmc_address {
	VCEB_GPMC_A1 = 0,
	VCEB_GPMC_A9 = 1,
};

struct vcebfb_platform_data {
	enum vcebfb_display_type display_type;
	enum vcebfb_gpmc_address gpmc_address;
};


/*
 * enum of the RGB buffer formats supported
 */
typedef enum
{
   VCEB_RBG_FORMAT_MIN = 0,

   //Note - all RGB formats are descibed in little endian
   //where n[0] is the lowest physical address
   VCEB_RBG_FORMAT_RGB565, //R[0], G[1+0], B[1]
   VCEB_RBG_FORMAT_RGB888, //R[0], G[1], B[2]
   VCEB_RBG_FORMAT_RGBA8888, //R[0], G[1], B[2], A[3]
   VCEB_RBG_FORMAT_BGRA8888, //R[0], G[1], B[2], A[3]

   VCEB_RBG_FORMAT_MAX
   
} VCEB_RBG_FORMAT_T;

//alignment options for putting the overlay on the screen
typedef enum
{
   VCEB_ALIGN_MIN = 0,

   VCEB_ALIGN_CENTRE,
   VCEB_ALIGN_STRETCH_FULLSCREEN,
   VCEB_ALIGN_BOTTOM_RIGHT,

   VCEB_ALIGN_MAX
   
} VCEB_ALIGN_T;

extern int32_t vceb_framebuffer_overlay_set(const void * const data,
					const VCEB_RBG_FORMAT_T rbg_format,
					const uint32_t transparent_colour,
					const uint16_t width,
					const uint16_t height,
					const VCEB_ALIGN_T alignment);

/*
 * function to register a callback that fires whenever
 * framebuffer needs an update
 */

struct vmcs_fb_ops {
	int (*open) (void *callback_param, uint32_t screen,
				struct fb_info *info);
	void (*release) (void *callback_param, uint32_t screen);
	int (*update) (void *fb_data, void *callback_param, uint32_t screen,
			struct fb_info *info);
};

extern int vceb_fb_add_screen(uint32_t screen);

extern void vceb_fb_remove_screen(uint32_t screen);

extern int vceb_fb_register_callbacks(uint32_t screen, struct vmcs_fb_ops *ops,
				void *callback_param);

extern void vceb_fb_bus_connected(uint32_t keep_vmcs_res);

extern int32_t vceb_initialise(const uint32_t noreset);

/*
extern int32_t vceb_initialise(const uint32_t noreset,
				enum vcebfb_display_type display_type,
				enum vcebfb_gpmc_address gpmc_address);
*/

extern int32_t vceb_framebuffer_overlay_enable(const uint32_t enable);

#endif /* VCEB_FB_H_ */
