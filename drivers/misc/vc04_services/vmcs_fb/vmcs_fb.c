/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

/*
 * Description:
 *    This is the videocore framebuffer driver. A framebuffer device for each
 *    of the supported screens is registered with the Linux system. The actual
 *    allocation of the framebuffer memory is not done until someone invokes
 *    the open command on the framebuffer device. This driver uses the
 *    framebuffer service, which provides (but not limited) the following:
 *       - information (resolution, bpp) for each attached screen
 *       - allocation of memory for framebuffer on videocore
 *       - panning/swapping of framebuffers
 *    Since the allocation is not done until the device open is invoked, users
 *    can modify certain parameters that affects the framebuffer via proc
 *    entries. The following properties can be modified:
 *       - alpha per pixel
 *       - default alpha (if alpha per pixel is not used)
 *       - h/w scaling
 *       - resolution override (this is the resolution reported to linux, which
 *         can be smaller or greater than the actual resolution of the screen)
 *       - z-ordering of the framebuffer
 *
 * Notes:
 *    1. Terminology:
 *       - display: display device that has one or more screens attached
 *       - screen: attached to a display that shows actual content, e.g. LCD
 *    2. Currently only one instance of videocore is supported
 *    3. STR not yet supported
 *    4. Rotation not yet supported
 *    5. TODO: Add a 'info' proc entry to show number of active users, current
 *       resolution, memory address/length, dev path (/dev/fb0), etc.
 */

// ---- Include Files --------------------------------------------------------

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/mm.h>

#include <mach/io_map.h>

//#include <linux/videocore/vc_boot_mode.h>

#include "interface/vcos/vcos.h"
#include "vc_vchi_fb.h"

// ---- Private Constants and Types ------------------------------------------

// Uncomment the following line to enable debug messages
//#define ENABLE_LOG_DBG

// Logging macros (for remapping to other logging mechanisms, i.e., vcos_log)
#ifdef ENABLE_LOG_DBG
#define LOG_DBG(  fmt, arg... )  printk( KERN_INFO "[D] " fmt "\n", ##arg )
#else
#define LOG_DBG(  fmt, arg... )
#endif
#define LOG_INFO( fmt, arg... )  printk( KERN_INFO "[I] " fmt "\n", ##arg )
#define LOG_ERR(  fmt, arg... )  printk( KERN_ERR  "[E] " fmt "\n", ##arg )

//#define VC_ROUND_UP_WH(wh)  (((wh) + 15) & ~15)

//TODO add to linux.config!!
#define CONFIG_FB_VC_DEFAULT_SCREEN_WIDTH     (800)
#define CONFIG_FB_VC_DEFAULT_SCREEN_HEIGHT    (480)
#define CONFIG_FB_VC_DEFAULT_BPP              (32)
#define CONFIG_FB_VC_NUM_FRAMES               (2)
#define CONFIG_FB_VC_DEFAULT_ALPHA            (255)
#define CONFIG_FB_VC_DEFAULT_ALPHA_PER_PIXEL  (1)
#define CONFIG_FB_VC_DEFAULT_HW_SCALE         (1)
#define CONFIG_FB_VC_DEFAULT_Z_ORDER          (50)

// Default values for framebuffer creation modifiable parameters
#define DEFAULT_ALPHA            (255)
#define DEFAULT_ALPHA_PER_PIXEL  (1)
#define DEFAULT_KEEP_RESOURCE    (0)
#define DEFAULT_SCALE            (1)
#define DEFAULT_Z_ORDER          (50)

typedef struct
{
   VC_FB_SCRN       scrn;

   uint32_t         user_cnt;          // Number of active users of this screen
   VCOS_MUTEX_T     user_cnt_mutex;    // Mutex to protect user count
   struct fb_info   fb_info;           // Kernel framebuffer info
   uint32_t         res_handle;        // Videocore resource handle
   uint32_t         cmap[16];          // Color map

   VCOS_CFG_ENTRY_T fb_cfg_directory;  // Per framebuffer device proc directory

   // Modifiable paramters for framebuffer creation (upon fb_open call)
   uint32_t         alpha;             // Alpha value to use when alpha_per_pixel=0
   uint32_t         alpha_per_pixel;   // 1 = Each pixel has its own alpha value
   uint32_t         keep_resource;     // Keep resource open even if release is called
   uint32_t         scale;             // Scale the image to fit the screen
   uint32_t         z_order;           // Z-order of framebuffer
   uint32_t         width_override;    // Width override (resolution override)
   uint32_t         height_override;   // Height override (resolution override)

   // Proc entries corresponding to the modifiable parameters
   VCOS_CFG_ENTRY_T alpha_cfg_entry;
   VCOS_CFG_ENTRY_T alpha_per_pixel_cfg_entry;
   VCOS_CFG_ENTRY_T keep_resource_cfg_entry;
   VCOS_CFG_ENTRY_T res_override_cfg_entry;
   VCOS_CFG_ENTRY_T scale_cfg_entry;
   VCOS_CFG_ENTRY_T z_order_cfg_entry;

} SCRN_INFO_T;

typedef struct
{
   VC_VCHI_FB_HANDLE_T fb_handle;
   SCRN_INFO_T        *scrn_info[VC_FB_SCRN_MAX];
   VCOS_CFG_ENTRY_T    cfg_directory;

} FB_STATE_T;

// ---- Private Variables ----------------------------------------------------

static FB_STATE_T *fb_state;

// Constant strings for the proc entries
static const char *fb_cfg_dir_name[VC_FB_SCRN_MAX] = {
   "0",
   "1",
};

// ---- Private Function Prototypes ------------------------------------------
// ---- Private Functions ----------------------------------------------------

static void generic_bool_cfg_entry_show( VCOS_CFG_BUF_T buf,
                                         void *data )
{
   vcos_assert( data != NULL );

	vcos_cfg_buf_printf( buf, "%u\n", !!*(uint32_t *)data );
}

static void generic_bool_cfg_entry_parse( VCOS_CFG_BUF_T buf,
                                          void *data )
{
   uint32_t *val = data;

   vcos_assert( data != NULL );

   *val = !!(simple_strtoul( vcos_cfg_buf_get_str( buf ), NULL, 10 ));
}

static void generic_uint_cfg_entry_show( VCOS_CFG_BUF_T buf,
                                         void *data )
{
   vcos_assert( data != NULL );

	vcos_cfg_buf_printf( buf, "%u\n", *(uint32_t *)data );
}

static void generic_uint_cfg_entry_parse( VCOS_CFG_BUF_T buf,
                                          void *data )
{
   uint32_t *val = data;

   vcos_assert( data != NULL );

   *val = simple_strtoul( vcos_cfg_buf_get_str( buf ), NULL, 10 );
}

static void alpha_cfg_entry_parse( VCOS_CFG_BUF_T buf,
                                   void *data )
{
   uint32_t *alpha = data;
   uint32_t val;

   vcos_assert( data != NULL );

   val = simple_strtoul( vcos_cfg_buf_get_str( buf ), NULL, 10 );
   *alpha = val < 255 ? val : 255;
}

static void res_override_cfg_entry_show( VCOS_CFG_BUF_T buf,
                                         void *data )
{
   SCRN_INFO_T *scrn_info = data;

   vcos_assert( data != NULL );

	vcos_cfg_buf_printf( buf, "%ux%u\n", scrn_info->width_override,
                        scrn_info->height_override );
}

static void res_override_cfg_entry_parse( VCOS_CFG_BUF_T buf,
                                          void *data )
{
   SCRN_INFO_T *scrn_info = data;
   uint32_t width;
   uint32_t height;

   vcos_assert( data != NULL );

   if ( sscanf( vcos_cfg_buf_get_str( buf ), "%ux%u", &width, &height ) != 2 )
   {
      LOG_ERR( "%s: invalid override resolution", __func__ );
   }
   else
   {
      scrn_info->width_override = width;
      scrn_info->height_override = height;
   }
}

static inline SCRN_INFO_T *to_scrn_info( struct fb_info *fb_info )
{
	return container_of( fb_info, SCRN_INFO_T, fb_info );
}

static inline uint32_t convert_bitfield( int val,
                                         struct fb_bitfield *bf )
{
	unsigned int mask = ( 1 << bf->length ) - 1;

	return ( val >> ( 16 - bf->length ) & mask ) << bf->offset;
}

static int vc_fb_get_info( SCRN_INFO_T *scrn_info )
{
   int ret;
   int32_t success;
   VC_FB_SCRN_INFO_T info;

   LOG_DBG( "%s: start (fb_info=0x%p)", __func__, fb_info );

   // Get the screen info from the framebuffer service
   success = vc_vchi_fb_get_scrn_info( fb_state->fb_handle, scrn_info->scrn,
                                       &info );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to get info for screen %u (success=%d)", __func__,
               scrn_info->scrn, success );

      ret = -EPERM;
      goto out;
   }
   else if (( info.width == 0 ) || ( info.height == 0 ))
   {
      LOG_DBG( "%s: could not get info for screen %u - using defaults",
                __func__, scrn_info->scrn );

      info.width = CONFIG_FB_VC_DEFAULT_SCREEN_WIDTH;
      info.height = CONFIG_FB_VC_DEFAULT_SCREEN_HEIGHT;
   }

   if ( info.bits_per_pixel == 0 )
   {
      info.bits_per_pixel = CONFIG_FB_VC_DEFAULT_BPP;
   }

   // Apply any overrides here
   if ( scrn_info->width_override != 0 )
   {
      info.width = scrn_info->width_override;
   }
   if ( scrn_info->height_override != 0 )
   {
      info.height = scrn_info->height_override;
   }

   LOG_DBG( "%s: screen %u: %ux%u, %u bpp", __func__, scrn_info->scrn,
            info.width, info.height, info.bits_per_pixel );

   if (( info.width % 16 ) || ( info.height % 16 ))
   {
      // Videocore needs the dimensions to be a multiple of 16
      info.width &= ~15;
      info.height &= ~15;

      LOG_DBG( "%s: screen %u: adjusted to %ux%u, %u bpp", __func__,
               scrn_info->scrn, info.width, info.height, info.bits_per_pixel );
   }

   // Save the info into struct fb_var_screeninfo
   scrn_info->fb_info.var.xres           = info.width;
   scrn_info->fb_info.var.yres           = info.height;
   scrn_info->fb_info.var.xres_virtual   = info.width;
   scrn_info->fb_info.var.yres_virtual   = info.height * CONFIG_FB_VC_NUM_FRAMES;
	scrn_info->fb_info.var.bits_per_pixel = info.bits_per_pixel;
   scrn_info->fb_info.var.activate       = FB_ACTIVATE_NOW;
	scrn_info->fb_info.var.height         = info.height;
	scrn_info->fb_info.var.width	        = info.width;

	if ( scrn_info->fb_info.var.bits_per_pixel == 16 )
	{
	   scrn_info->fb_info.var.red.offset   = 11;
	   scrn_info->fb_info.var.red.length   = 5;
	   scrn_info->fb_info.var.green.offset = 5;
	   scrn_info->fb_info.var.green.length = 6;
	   scrn_info->fb_info.var.blue.offset  = 0;
	   scrn_info->fb_info.var.blue.length  = 5;
	}
	else
	{
	   scrn_info->fb_info.var.red.offset    = 16;
	   scrn_info->fb_info.var.red.length    = 8;
	   scrn_info->fb_info.var.green.offset  = 8;
	   scrn_info->fb_info.var.green.length  = 8;
	   scrn_info->fb_info.var.blue.offset   = 0;
	   scrn_info->fb_info.var.blue.length   = 8;
	   scrn_info->fb_info.var.transp.offset = 24;
	   scrn_info->fb_info.var.transp.length = 8;
	}

   ret = fb_set_var( &scrn_info->fb_info, &scrn_info->fb_info.var );
   if ( ret != 0 )
   {
      LOG_ERR( "%s: fb_set_var failed (ret=%d)", __func__, ret );

      goto out;
   }

out:
   LOG_DBG( "%s: end (ret=%d)", __func__, ret );

   return ret;
}

static int vc_fb_open( struct fb_info *fb_info,
                       int user )
{
   int ret = 0;
   SCRN_INFO_T *scrn_info = to_scrn_info( fb_info );

   LOG_DBG( "%s: start (fb_info=0x%p, user=%d)", __func__, fb_info, user );

   vcos_mutex_lock( &scrn_info->user_cnt_mutex );

   LOG_DBG( "%s: scrn_info->user_cnt=%u", __func__, scrn_info->user_cnt );

   // Only allocate the framebuffer if its the first user AND we do not already
   // have one allocated
   if (( scrn_info->user_cnt == 0 ) && ( scrn_info->res_handle == 0 ))
   {
      int32_t success;
      VC_FB_ALLOC_T alloc;
      VC_FB_ALLOC_RESULT_T alloc_result;
      uint32_t vc_addr;

      ret = vc_fb_get_info( scrn_info );
      if ( ret != 0 )
      {
         LOG_ERR( "%s: failed to get info for screen %u (ret=%d)", __func__,
                  scrn_info->scrn, ret );

         goto out;
      }

      alloc.scrn = scrn_info->scrn;
      alloc.width = fb_info->var.xres;
      alloc.height = fb_info->var.yres;
      alloc.bits_per_pixel = fb_info->var.bits_per_pixel;
      alloc.num_frames = CONFIG_FB_VC_NUM_FRAMES;
      alloc.layer = scrn_info->z_order;
      alloc.alpha_per_pixel = scrn_info->alpha_per_pixel;
      alloc.default_alpha = scrn_info->alpha;
      alloc.scale = scrn_info->scale;

      LOG_INFO( "%s: allocating framebuffer for screen %u", __func__,
                alloc.scrn );
      LOG_INFO( "%s:\t%ux%u, bpp=%u, num_frames=%u, z-order=%u, scale=%u",
                __func__, alloc.width, alloc.height, alloc.bits_per_pixel,
                alloc.num_frames, alloc.layer, alloc.scale );
      LOG_INFO( "%s:\talpha_per_pixel=%u, default_alpha=%u", __func__,
                alloc.alpha_per_pixel, alloc.default_alpha );

      // Allocate memory for the framebuffer
      success = vc_vchi_fb_alloc( fb_state->fb_handle, &alloc, &alloc_result );
      if (( success != 0 ) || ( alloc_result.res_handle == 0 ))
      {
         LOG_ERR( "%s: failed to allocate framebuffer on videocore (success=%d)",
                  __func__, success );

         ret = -ENOMEM;
         goto out;
      }

      LOG_DBG( "%s: alloc_result: res_handle=0x%08x, res_mem=0x%p, line_bytes=%u, frame_bytes=%u",
               __func__, alloc_result.res_handle, alloc_result.res_mem,
               alloc_result.line_bytes, alloc_result.frame_bytes );

      // Save the resource handle
      scrn_info->res_handle = alloc_result.res_handle;

      // Mask out the top two bits of the videocore address to get the offset;
      vc_addr = (uint32_t)alloc_result.res_mem & 0x3FFFFFFF;

      // Then use the offset to calculate the virtual and physical addresses
      fb_info->screen_base = (void *)(vc_addr + KONA_VC_EMI);
      fb_info->fix.smem_start = (unsigned long)(vc_addr + VC_EMI);
      fb_info->fix.smem_len = alloc_result.frame_bytes * alloc.num_frames;
      fb_info->fix.line_length = alloc_result.line_bytes;

      LOG_DBG( "%s: screen_base=0x%p, smem_start=0x%08x, smem_len=%u, line_length=%u",
               __func__, fb_info->screen_base, (uint32_t)fb_info->fix.smem_start,
               fb_info->fix.smem_len, fb_info->fix.line_length );
      LOG_DBG( "%s: virt_to_phys=0x%u", __func__,
               (uint32_t)virt_to_phys(fb_info->screen_base));

   }

   // Increase the user count by one
   scrn_info->user_cnt++;

out:
   vcos_mutex_unlock( &scrn_info->user_cnt_mutex );

   LOG_DBG( "%s: end (ret=%d)", __func__, ret );

   return ret;
}

static int vc_fb_release( struct fb_info *fb_info,
                          int user )
{
   int ret = 0;
   SCRN_INFO_T *scrn_info = to_scrn_info( fb_info );

   LOG_DBG( "%s: start (fb_info=0x%p, user=%d)", __func__, fb_info, user );

   vcos_mutex_lock( &scrn_info->user_cnt_mutex );

   LOG_DBG( "%s: scrn_info->user_cnt=%u", __func__, scrn_info->user_cnt );

   // Only free the framebuffer if there are no more users AND we do not want
   // to keep the resource around
   if (( --scrn_info->user_cnt == 0 ) && ( scrn_info->keep_resource == 0 ))
   {
      int32_t success;

      LOG_DBG( "%s: freeing videocore framebuffer", __func__ );

      fb_info->screen_base = NULL;
      fb_info->fix.smem_start = 0;
      fb_info->fix.smem_len = 0;

      success = vc_vchi_fb_free( fb_state->fb_handle, scrn_info->res_handle );
      if ( success != 0 )
      {
         LOG_ERR( "%s: failed to free framebuffer (success=%d)", __func__,
                  success );

         // Even if we failed to release it, we should continue on as if it
         // succeeded - this might lead to memory leaks on the videocore!!
      }

      scrn_info->res_handle = 0;
   }

   vcos_mutex_unlock( &scrn_info->user_cnt_mutex );

   LOG_DBG( "%s: end (ret=%d)", __func__, ret );

   return ret;
}

static int vc_fb_check_var( struct fb_var_screeninfo *var,
                            struct fb_info *fb_info )
{
   int ret = 0;

   LOG_DBG( "%s: start (var=0x%p, fb_info=0x%p)", __func__, var, fb_info );

   // Check for parameters that we cannot change
   if (( var->xoffset != fb_info->var.xoffset ) ||
       ( var->bits_per_pixel != fb_info->var.bits_per_pixel ) ||
       ( var->grayscale != fb_info->var.grayscale ))
   {
      ret = -EINVAL;
      goto out;
   }

   if (( var->xres > fb_info->var.xres ) || ( var->yres > fb_info->var.yres ))
   {
      LOG_INFO( "%s: request resolution %ux%u is larger than supported %ux%u",
                __func__, var->xres, var->yres, fb_info->var.xres,
                fb_info->var.yres );

      ret = -EINVAL;
      goto out;
   }

out:
   LOG_DBG( "%s: end (ret=%d)", __func__, ret );

   return ret;
}

static int vc_fb_set_par( struct fb_info *fb_info )
{
   int ret = 0;

   LOG_DBG( "%s: start (fb_info=0x%p)", __func__, fb_info );

   //TODO Support rotation

   LOG_DBG( "%s: end (ret=%d)", __func__, ret );

   return ret;
}

static int vc_fb_setcolreg( unsigned int regno,
                            unsigned int red,
                            unsigned int green,
                            unsigned int blue,
                            unsigned int transp,
                            struct fb_info *fb_info )
{
   int ret = 0;
   SCRN_INFO_T *scrn_info = to_scrn_info( fb_info );

   LOG_DBG( "%s: start (regno=%u, red=%u, green=%u, blue=%u, transp=%u, fb_info=0x%p)",
            __func__, regno, red, green, blue, transp, fb_info );

   // We only support 16 color registers
   if ( regno < 16 )
   {
      scrn_info->cmap[regno] =
         convert_bitfield( red, &scrn_info->fb_info.var.red ) |
         convert_bitfield( green, &scrn_info->fb_info.var.green ) |
         convert_bitfield( blue, &scrn_info->fb_info.var.blue ) |
         convert_bitfield( transp, &scrn_info->fb_info.var.transp );
   }
   else if ( regno > 255 )
   {
      LOG_INFO( "%s: invalid color register number %u", __func__, regno );

      ret = 1;
   }

   LOG_DBG( "%s: end (ret=%d)", __func__, ret );

   return ret;
}

static int vc_fb_pan_scrn( struct fb_var_screeninfo *var,
                           struct fb_info *fb_info )
{
   int ret = 0;
   int32_t success;
   SCRN_INFO_T *scrn_info = to_scrn_info( fb_info );

   LOG_DBG( "%s: start (var=0x%p, fb_info=0x%p)", __func__, var, fb_info );

   success = vc_vchi_fb_pan( fb_state->fb_handle, scrn_info->res_handle,
                             var->yoffset );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to pan (success=%d)", __func__, success );

      ret = -EPERM;
   }

   LOG_DBG( "%s: end (ret=%d)", __func__, ret );

   return ret;
}

static struct fb_ops vc_fb_ops = {
   .owner          = THIS_MODULE,
   .fb_open        = vc_fb_open,
   .fb_release     = vc_fb_release,
//   .fb_read        = ,
//   .fb_write       = ,
   .fb_check_var   = vc_fb_check_var,
   .fb_set_par     = vc_fb_set_par,
   .fb_setcolreg   = vc_fb_setcolreg,
//   .fb_blank       = ,
   .fb_pan_display = vc_fb_pan_scrn,
   .fb_fillrect    = cfb_fillrect,
   .fb_copyarea    = cfb_copyarea,
   .fb_imageblit   = cfb_imageblit,
//   .fb_cursor      = ,
//   .fb_rotate      = ,
};

static int vc_fb_create_per_scrn_proc_entries( SCRN_INFO_T *scrn_info )
{
   int ret;
   VCOS_STATUS_T status;

   LOG_DBG( "%s: start (scrn_info=0x%p)", __func__, scrn_info );

   // First create a 'fb<x>' proc directory
   status = vcos_cfg_mkdir( &scrn_info->fb_cfg_directory,
                            &fb_state->cfg_directory,
                            fb_cfg_dir_name[scrn_info->scrn] );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create proc directory entry (status=%d)",
               __func__, status );

      ret = -EPERM;
      goto out;
   }

   // Now create all the proc entries for modifiable parameters
   status = vcos_cfg_create_entry( &scrn_info->alpha_cfg_entry,
                                   &scrn_info->fb_cfg_directory,
                                   "alpha",
                                   generic_uint_cfg_entry_show,
                                   alpha_cfg_entry_parse,
                                   &scrn_info->alpha );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create proc entry (status=%d)", __func__,
               status );

      ret = -EPERM;
      goto err_remove_fb_cfg_directory;
   }

   status = vcos_cfg_create_entry( &scrn_info->alpha_per_pixel_cfg_entry,
                                   &scrn_info->fb_cfg_directory,
                                   "alpha_per_pixel",
                                   generic_bool_cfg_entry_show,
                                   generic_bool_cfg_entry_parse,
                                   &scrn_info->alpha_per_pixel );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create proc entry (status=%d)", __func__,
               status );

      ret = -EPERM;
      goto err_remove_alpha_cfg_entry;
   }

   status = vcos_cfg_create_entry( &scrn_info->keep_resource_cfg_entry,
                                   &scrn_info->fb_cfg_directory,
                                   "keep_resource",
                                   generic_bool_cfg_entry_show,
                                   generic_bool_cfg_entry_parse,
                                   &scrn_info->keep_resource );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create proc entry (status=%d)", __func__,
               status );

      ret = -EPERM;
      goto err_remove_alpha_per_pixel_cfg_entry;
   }

   status = vcos_cfg_create_entry( &scrn_info->res_override_cfg_entry,
                                   &scrn_info->fb_cfg_directory,
                                   "res_override",
                                   res_override_cfg_entry_show,
                                   res_override_cfg_entry_parse,
                                   scrn_info );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create proc entry (status=%d)", __func__,
               status );

      ret = -EPERM;
      goto err_remove_keep_res_cfg_entry;
   }

   status = vcos_cfg_create_entry( &scrn_info->scale_cfg_entry,
                                   &scrn_info->fb_cfg_directory,
                                   "scale",
                                   generic_bool_cfg_entry_show,
                                   generic_bool_cfg_entry_parse,
                                   &scrn_info->scale );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create proc entry (status=%d)", __func__,
               status );

      ret = -EPERM;
      goto err_remove_res_override_cfg_entry;
   }

   status = vcos_cfg_create_entry( &scrn_info->z_order_cfg_entry,
                                   &scrn_info->fb_cfg_directory,
                                   "z_order",
                                   generic_uint_cfg_entry_show,
                                   generic_uint_cfg_entry_parse,
                                   &scrn_info->z_order );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create proc entry (status=%d)", __func__,
               status );

      ret = -EPERM;
      goto err_remove_scale_cfg_entry;
   }

   ret = 0;
   goto out;

err_remove_scale_cfg_entry:
   vcos_cfg_remove_entry( &scrn_info->scale_cfg_entry );

err_remove_res_override_cfg_entry:
   vcos_cfg_remove_entry( &scrn_info->res_override_cfg_entry );

err_remove_keep_res_cfg_entry:
   vcos_cfg_remove_entry( &scrn_info->keep_resource_cfg_entry );

err_remove_alpha_per_pixel_cfg_entry:
   vcos_cfg_remove_entry( &scrn_info->alpha_per_pixel_cfg_entry );

err_remove_alpha_cfg_entry:
   vcos_cfg_remove_entry( &scrn_info->alpha_cfg_entry );

err_remove_fb_cfg_directory:
   vcos_cfg_remove_entry( &scrn_info->fb_cfg_directory );

out:
   LOG_DBG( "%s: end (ret=%d)", __func__, ret );

   return ret;
}

static int vc_fb_remove_per_scrn_proc_entries( SCRN_INFO_T *scrn_info )
{
   int ret = 0;
   VCOS_STATUS_T status;

   LOG_DBG( "%s: start (scrn_info=0x%p)", __func__, scrn_info );

   status = vcos_cfg_remove_entry( &scrn_info->alpha_cfg_entry );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to remove proc entry (status=%d)", __func__,
               status );

      ret = -EPERM;
   }
   status = vcos_cfg_remove_entry( &scrn_info->alpha_per_pixel_cfg_entry );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to remove proc entry (status=%d)", __func__,
               status );

      ret = -EPERM;
   }
   status = vcos_cfg_remove_entry( &scrn_info->keep_resource_cfg_entry );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to remove proc entry (status=%d)", __func__,
               status );

      ret = -EPERM;
   }
   status = vcos_cfg_remove_entry( &scrn_info->res_override_cfg_entry );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to remove proc entry (status=%d)", __func__,
               status );

      ret = -EPERM;
   }
   status = vcos_cfg_remove_entry( &scrn_info->scale_cfg_entry );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to remove proc entry (status=%d)", __func__,
               status );

      ret = -EPERM;
   }
   status = vcos_cfg_remove_entry( &scrn_info->z_order_cfg_entry );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to remove proc entry (status=%d)", __func__,
               status );

      ret = -EPERM;
   }
   status = vcos_cfg_remove_entry( &scrn_info->fb_cfg_directory );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to remove proc directory (status=%d)", __func__,
               status );

      ret = -EPERM;
   }

   LOG_DBG( "%s: end (ret=%d)", __func__, ret );

   return ret;
}

static int vc_fb_create_framebuffer( VC_FB_SCRN scrn )
{
   int ret;
   SCRN_INFO_T *scrn_info;
   VCOS_STATUS_T status;

   LOG_DBG( "%s: start (scrn=%u)", __func__, scrn );

   vcos_assert( scrn < VC_FB_SCRN_MAX );

   if ( fb_state->scrn_info[scrn] != NULL )
   {
      LOG_ERR( "%s: framebuffer already created for screen %u", __func__,
               scrn );

      ret = -EEXIST;
      goto out;
   }

   // Allocate memory for the screen info
   scrn_info = vcos_kcalloc( 1, sizeof( *scrn_info ), __func__ );
   if ( scrn_info == NULL )
   {
      LOG_ERR( "%s: failed to allocate memory for screen info", __func__ );

      ret = -ENOMEM;
      goto out;
   }

   scrn_info->scrn = scrn;

   // Create the user count mutex
   status = vcos_mutex_create( &scrn_info->user_cnt_mutex, "vc_fb_mutex" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create mutex (status=%d)", __func__, status );

      ret = -EPERM;
      goto err_free_mem;
   }

   /*
    * Fill in most of the information in fb_info, and leave out the following
    * information that will become available later on when we ask for it from
    * the framebuffer service:
    *    - framebuffer memory address and length
    *    - resolution and bpp
    * We do not ask the framebuffer service for that information now because we
    * want the allow the user to override the resolution via the proc entries
    * prior to invoking fb_open.
    */

   scrn_info->fb_info.fbops = &vc_fb_ops;
   scrn_info->fb_info.flags = FBINFO_FLAG_DEFAULT;
   scrn_info->fb_info.pseudo_palette = scrn_info->cmap;

   // struct fb_fix_screeninfo
   strncpy( scrn_info->fb_info.fix.id, "vc_fb",
            sizeof( scrn_info->fb_info.fix.id ));
   scrn_info->fb_info.fix.type      = FB_TYPE_PACKED_PIXELS;
   scrn_info->fb_info.fix.visual    = FB_VISUAL_TRUECOLOR;
   scrn_info->fb_info.fix.xpanstep  = 0;
   scrn_info->fb_info.fix.ypanstep  = 1;
   scrn_info->fb_info.fix.ywrapstep = 0;
   scrn_info->fb_info.fix.accel     = FB_ACCEL_NONE;

   // struct fb_var_screeninfo
   scrn_info->fb_info.var.grayscale = 0;
   scrn_info->fb_info.var.nonstd    = 0;
   scrn_info->fb_info.var.activate  = FB_ACTIVATE_NOW;
   scrn_info->fb_info.var.rotate    = 0;
   scrn_info->fb_info.var.vmode     = FB_VMODE_NONINTERLACED;

   // Register framebuffer with the kernel
   ret = register_framebuffer( &scrn_info->fb_info );
   if ( ret != 0 )
   {
      LOG_ERR( "%s: register_framebuffer failed (ret=%d)", __func__, ret );

      goto err_delete_mutex;
   }

   // Create the per screen (framebuffer device) proc entries
   ret = vc_fb_create_per_scrn_proc_entries( scrn_info );
   if ( ret != 0 )
   {
      LOG_ERR( "%s: failed to create proc entries (ret=%d)", __func__, ret );

      goto err_unregister_framebuffer;
   }

   // Set the default values for the framebuffer creation modifiable parameters
   scrn_info->alpha           = DEFAULT_ALPHA;
   scrn_info->alpha_per_pixel = DEFAULT_ALPHA_PER_PIXEL;
   scrn_info->keep_resource   = DEFAULT_KEEP_RESOURCE;
   scrn_info->scale           = DEFAULT_SCALE;
   scrn_info->z_order         = DEFAULT_Z_ORDER;

   // Everything is good to go!
   fb_state->scrn_info[scrn] = scrn_info;

   goto out;

err_unregister_framebuffer:
   unregister_framebuffer( &scrn_info->fb_info );

err_delete_mutex:
   vcos_mutex_delete( &scrn_info->user_cnt_mutex );

err_free_mem:
   vcos_kfree( scrn_info );

out:
   LOG_DBG( "%s: end (ret=%d)", __func__, ret );

   return ret;
}

static int vc_fb_remove_framebuffer( VC_FB_SCRN scrn )
{
   int ret;
   SCRN_INFO_T *scrn_info;

   LOG_DBG( "%s: start (scrn=%u)", __func__, scrn );

   vcos_assert( scrn < VC_FB_SCRN_MAX );

   scrn_info = fb_state->scrn_info[scrn];
   if ( scrn_info == NULL )
   {
      LOG_DBG( "%s: no framebuffer device for screen %u", __func__, scrn );

      ret = 0;
      goto out;
   }

   // Remove per screen (framebuffer device) proc entries
   vc_fb_remove_per_scrn_proc_entries( scrn_info );

   // Unregister framebuffer device with the system
   unregister_framebuffer( &scrn_info->fb_info );

   // Delete the user count mutex
   vcos_mutex_delete( &scrn_info->user_cnt_mutex );

   // Free the memory used for the screen info
   vcos_kfree( scrn_info );
   fb_state->scrn_info[scrn] = NULL;

out:
   LOG_DBG( "%s: end (ret=%d)", __func__, ret );

   return ret;
}

static int __init vc_fb_init( void )
{
   int ret;
   int i;
   VCHI_INSTANCE_T vchi_instance;
   VCHI_CONNECTION_T *vchi_connection;
   VCOS_STATUS_T status;

   LOG_INFO( "%s: start", __func__ );

#if 0
   if ( vc_boot_mode_skip() )
   {
      LOG_INFO( "%s: vc-boot-mode == skip - not initializing videocore",
                __func__ );

      ret = -ENODEV;
      goto out;
   }
#endif
   //TODO Check that there is at least one videocore instance

   // Allocate memory for the state structure
   fb_state = vcos_kcalloc( 1, sizeof( FB_STATE_T ), __func__ );
   if ( fb_state == NULL )
   {
      LOG_ERR( "%s: failed to allocate memory", __func__ );

      ret = -ENOMEM;
      goto out;
   }

   //TODO Call VCOS init somewhere central for entire system!

   // Initialize and create a VCHI connection
   ret = vchi_initialise( &vchi_instance );
   if ( ret != 0 )
   {
      LOG_ERR( "%s: failed to initialise VCHI instance (ret=%d)",
               __func__, ret );

      ret = -EIO;
      goto err_free_mem;
   }
   ret = vchi_connect( NULL, 0, vchi_instance );
   if ( ret != 0 )
   {
      LOG_ERR( "%s: failed to connect VCHI instance (ret=%d)",
               __func__, ret );

      ret = -EIO;
      goto err_free_mem;
   }

   // Initialize an instance of the framebuffer service
   fb_state->fb_handle = vc_vchi_fb_init( vchi_instance, &vchi_connection, 1 );
   if ( fb_state->fb_handle == NULL )
   {
      LOG_ERR( "%s: failed to initialize framebuffer service", __func__ );

      ret = -EPERM;
      goto err_free_mem;
   }

   // Create a proc directory entry
   status = vcos_cfg_mkdir( &fb_state->cfg_directory, NULL, "vc-fb" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create proc directory entry (status=%d)",
               __func__, status );

      ret = -EPERM;
      goto err_stop_fb_service;
   }

   // Create a framebuffer device for each screen
   for ( i = 0; i < VC_FB_SCRN_MAX; i++ )
   {
      ret = vc_fb_create_framebuffer( i );
      if ( ret != 0 )
      {
         LOG_ERR( "%s: failed to create framebuffer device for screen %u",
                  __func__, i );

         goto err_remove_framebuffer;
      }
   }

   // Done!

   goto out;

err_remove_framebuffer:
   for ( i = 0; i < VC_FB_SCRN_MAX; i++ )
   {
      vc_fb_remove_framebuffer( i );
   }

   vcos_cfg_remove_entry( &fb_state->cfg_directory );

err_stop_fb_service:
   vc_vchi_fb_stop( &fb_state->fb_handle );

err_free_mem:
   vcos_kfree( fb_state );

out:
   LOG_INFO( "%s: end (ret=%d)", __func__, ret );

   return ret;
}

static void __exit vc_fb_exit( void )
{
   int i;

   LOG_INFO( "%s: start", __func__ );

   // Remove framebuffer device for each screen
   for ( i = 0; i < VC_FB_SCRN_MAX; i++ )
   {
      vc_fb_remove_framebuffer( i );
   }

   // Remove the proc directory entry
   vcos_cfg_remove_entry( &fb_state->cfg_directory );

   // Stop the framebuffer service
   vc_vchi_fb_stop( &fb_state->fb_handle );

   // Free the memory for the state structure
   vcos_kfree( fb_state );

   LOG_INFO( "%s: end", __func__ );
}

module_init( vc_fb_init );
module_exit( vc_fb_exit );

MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "VC Framebuffer Driver" );
MODULE_LICENSE( "GPL v2" );
