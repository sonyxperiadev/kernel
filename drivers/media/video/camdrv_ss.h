#ifndef __CAMDRV_SS_H__
#define __CAMDRV_SS_H__

#include <linux/i2c.h>
#include <linux/device.h>
#include <asm/gpio.h>
#include <mach/kona.h>
#include <mach/rhea.h>
#include <asm/mach/map.h>
#include <linux/power_supply.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#include <linux/broadcom/bcm59055-power.h>
#include <linux/clk.h>
#include <linux/bootmem.h>
#include <plat/pi_mgr.h>
#include <linux/sysdev.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/completion.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <linux/videodev2_brcm.h>


struct camdrv_ss_framesize {
	u32 index;
	u32 width;
	u32 height;
};

int camdrv_ss_i2c_write_2_bytes(struct i2c_client *client,  unsigned short data);

int camdrv_ss_i2c_write_4_bytes(struct i2c_client *client, unsigned short subaddr, unsigned short data);
int camdrv_ss_i2c_read_4_bytes(struct i2c_client *client, unsigned short subaddr, unsigned short *data);

int camdrv_ss_i2c_set_config_register(struct i2c_client *client,
					regs_t reg_buffer[],
					int num_of_regs,
					char *name);


enum camdrv_ss_cam_light {
	CAM_LOW_LIGHT = 0,
	CAM_NORMAL_LIGHT,
	CAM_HIGH_LIGHT,
};

enum camdrv_ss_focus_result {
	CAMDRV_SS_AF_FOCUSED = 0,
	CAMDRV_SS_AF_FAILED,
	CAMDRV_SS_AF_SEARCHING,
};



struct camdrv_ss_sensor_cap {
/************************/
/***STRUCTURES**************/
/************************/
	char name[50];
	const struct camdrv_ss_framesize *supported_preview_framesize_list;
	const struct camdrv_ss_framesize *supported_capture_framesize_list ;
	const struct v4l2_fmtdesc *fmts;
	int rows_num_fmts;
	const struct v4l2_queryctrl *controls;
	int rows_num_controls;

	int supported_number_of_preview_sizes;
	int supported_number_of_capture_sizes;
	int default_pix_fmt;
	int default_mbus_pix_fmt;
	int register_size;


/*******************/
/* FUNCTION POINTERS */
/******************/

/* Drivers need to implement this mandatory */
	long (*thumbnail_ioctl)(struct v4l2_subdev *sd, unsigned int cmd, void *arg);
    int  (*enum_frameintervals)(struct v4l2_subdev *sd, struct v4l2_frmivalenum *fival);

/* Optional */
	int (*get_shutterspeed)(struct v4l2_subdev *sd);
	int (*get_iso_speed_rate)(struct v4l2_subdev *sd);
	int (*get_ae_stable_status)(struct v4l2_subdev *sd, struct v4l2_control *ctrl);
	int (*set_auto_focus)(struct v4l2_subdev *sd, struct v4l2_control *ctrl);
	int (*get_auto_focus_status)(struct v4l2_subdev *sd, struct v4l2_control *ctrl);
	int  (*set_touch_focus)(struct v4l2_subdev *sd, enum v4l2_touch_af touch_af, v4l2_touch_area *touch_area);
	int  (*get_touch_focus_status)(struct v4l2_subdev *sd, struct v4l2_control *ctrl);
	int (*AAT_flash_control)(struct v4l2_subdev *sd, int control_mode);
	int (*i2c_set_data_burst)(struct i2c_client *client, regs_t reg_buffer[], int num_of_regs);
	bool (*check_flash_needed)(struct v4l2_subdev *sd);
	int (*get_light_condition)(struct v4l2_subdev *sd, int *Result);
	int (*sensor_power)(int on);

/************************/
/* REGISTER TABLE SETTINGS */
/************************/
	const regs_t *init_regs;
	const regs_t *preview_camera_regs;

	/* snapshot mode */
	const regs_t *snapshot_normal_regs;
	const regs_t *snapshot_lowlight_regs;
	const regs_t *snapshot_highlight_regs;
	const regs_t *snapshot_nightmode_regs;
	const regs_t *snapshot_flash_on_regs;
	const regs_t *snapshot_af_preflash_on_regs;
	const regs_t *snapshot_af_preflash_off_regs;

	/*af*/
	const regs_t *af_macro_mode_regs;
	const regs_t *af_normal_mode_regs;
	const regs_t *single_af_start_regs;
	const regs_t *get_1st_af_search_status;
	const regs_t *get_2nd_af_search_status;
	const regs_t *single_af_stop_regs;

	/*effect*/
	const regs_t *effect_normal_regs;
	const regs_t *effect_negative_regs;
	const regs_t *effect_sepia_regs;
	const regs_t *effect_mono_regs;
	const regs_t *effect_aqua_regs;
	const regs_t *effect_sharpen_regs;
	const regs_t *effect_solarization_regs;
	const regs_t *effect_black_white_regs;

	/*wb*/
	const regs_t *wb_auto_regs;
	const regs_t *wb_sunny_regs;
	const regs_t *wb_cloudy_regs;
	const regs_t *wb_tungsten_regs;
	const regs_t *wb_fluorescent_regs;
	const regs_t *wb_cwf_regs;

	/*metering*/
	const regs_t *metering_matrix_regs;
	const regs_t *metering_center_regs;
	const regs_t *metering_spot_regs;

	/*EV*/
	const regs_t *ev_minus_4_regs;
	const regs_t *ev_minus_3_regs;
	const regs_t *ev_minus_2_regs;
	const regs_t *ev_minus_1_regs;
	const regs_t *ev_default_regs;
	const regs_t *ev_plus_1_regs;
	const regs_t *ev_plus_2_regs;
	const regs_t *ev_plus_3_regs;
	const regs_t *ev_plus_4_regs;

	/*contrast*/
	const regs_t *contrast_minus_2_regs;
	const regs_t *contrast_minus_1_regs;
	const regs_t *contrast_default_regs;
	const regs_t *contrast_plus_1_regs;
	const regs_t *contrast_plus_2_regs;

	/*sharpness*/
	const regs_t *sharpness_minus_3_regs;
	const regs_t *sharpness_minus_2_regs;
	const regs_t *sharpness_minus_1_regs;
	const regs_t *sharpness_default_regs;
	const regs_t *sharpness_plus_1_regs;
	const regs_t *sharpness_plus_2_regs;
	const regs_t *sharpness_plus_3_regs;

	/*saturation*/
	const regs_t *saturation_minus_2_regs;
	const regs_t *saturation_minus_1_regs;
	const regs_t *saturation_default_regs;
	const regs_t *saturation_plus_1_regs;
	const regs_t *saturation_plus_2_regs;

	/*zoom*/
	const regs_t *zoom_00_regs;
	const regs_t *zoom_01_regs;
	const regs_t *zoom_02_regs;
	const regs_t *zoom_03_regs;
	const regs_t *zoom_04_regs;
	const regs_t *zoom_05_regs;
	const regs_t *zoom_06_regs;
	const regs_t *zoom_07_regs;
	const regs_t *zoom_08_regs;

	/*scene mode*/
	const regs_t *scene_none_regs;
	const regs_t *scene_portrait_regs;
	const regs_t *scene_nightshot_regs;
	const regs_t *scene_backlight_regs;
	const regs_t *scene_landscape_regs;
	const regs_t *scene_sports_regs;
	const regs_t *scene_party_indoor_regs;
	const regs_t *scene_beach_snow_regs;
	const regs_t *scene_sunset_regs;
	const regs_t *scene_duskdawn_regs;
	const regs_t *scene_fall_color_regs;
	const regs_t *scene_fireworks_regs;
	const regs_t *scene_candle_light_regs;
	const regs_t *scene_text_regs;

	/*fps*/
	const regs_t *fps_auto_regs;
	const regs_t *fps_5_regs;
	const regs_t *fps_7_regs;
	const regs_t *fps_10_regs;
	const regs_t *fps_15_regs;
	const regs_t *fps_20_regs;
	const regs_t *fps_25_regs;
	const regs_t *fps_30_regs;
	const regs_t *fps_60_regs;
	const regs_t *fps_120_regs;

	/*quality*/
	const regs_t *quality_superfine_regs;
	const regs_t *quality_fine_regs;
	const regs_t *quality_normal_regs;
	const regs_t *quality_economy_regs;

	/*preview size */
	const regs_t *preview_size_176x144_regs;
	const regs_t *preview_size_320x240_regs;
	const regs_t *preview_size_352x288_regs;
	const regs_t *preview_size_640x480_regs;
	const regs_t *preview_size_704x576_regs;
	const regs_t *preview_size_720x480_regs;
	const regs_t *preview_size_800x480_regs;
	const regs_t *preview_size_800x600_regs;
	const regs_t *preview_size_1024x600_regs;
	const regs_t *preview_size_1024x768_regs;
	const regs_t *preview_size_1280x960_regs;
	const regs_t *preview_size_1600x960_regs;
	const regs_t *preview_size_1600x1200_regs;
	const regs_t *preview_size_2048x1232_regs;
	const regs_t *preview_size_2048x1536_regs;
	const regs_t *preview_size_2560x1920_regs;

	/*Capture size */
	const regs_t *capture_size_640x480_regs;
	const regs_t *capture_size_720x480_regs;
	const regs_t *capture_size_800x480_regs;
	const regs_t *capture_size_800x486_regs;
	const regs_t *capture_size_800x600_regs;
	const regs_t *capture_size_1024x600_regs;
	const regs_t *capture_size_1024x768_regs;
	const regs_t *capture_size_1280x960_regs;
	const regs_t *capture_size_1600x960_regs;
	const regs_t *capture_size_1600x1200_regs;
	const regs_t *capture_size_2048x1232_regs;
	const regs_t *capture_size_2048x1536_regs;
	const regs_t *capture_size_2560x1536_regs;
	const regs_t *capture_size_2560x1920_regs;

	/*pattern*/
	const regs_t *pattern_on_regs;
	const regs_t *pattern_off_regs;

	/* AE */
	const regs_t *ae_lock_regs;
	const regs_t *ae_unlock_regs;

	/* AWB */
	const regs_t *awb_lock_regs;
	const regs_t *awb_unlock_regs;


	/* NO OF ROWS OF EACH REGISTER SETTING */
	int  rows_num_init_regs;
	int  rows_num_preview_camera_regs;

	/* snapshot mode */
	int  rows_num_snapshot_normal_regs;
	int  rows_num_snapshot_lowlight_regs;
	int  rows_num_snapshot_highlight_regs;
	int  rows_num_snapshot_nightmode_regs;
	int  rows_num_snapshot_flash_on_regs;
	int  rows_num_snapshot_af_preflash_on_regs;
	int  rows_num_snapshot_af_preflash_off_regs;

	/*af*/
	int  rows_num_af_macro_mode_regs;
	int  rows_num_af_normal_mode_regs;
	int  rows_num_single_af_start_regs;
	int  rows_num_get_1st_af_search_status;
	int  rows_num_get_2nd_af_search_status;
	int  rows_num_single_af_stop_regs;

	/*effect*/
	int  rows_num_effect_normal_regs;
	int  rows_num_effect_negative_regs;
	int  rows_num_effect_sepia_regs;
	int  rows_num_effect_mono_regs;
	int  rows_num_effect_aqua_regs;
	int  rows_num_effect_sharpen_regs;
	int  rows_num_effect_solarization_regs;
	int  rows_num_effect_black_white_regs;

	/*wb*/
	int  rows_num_wb_auto_regs;
	int  rows_num_wb_sunny_regs;
	int  rows_num_wb_cloudy_regs;
	int  rows_num_wb_tungsten_regs;
	int  rows_num_wb_fluorescent_regs;
	int  rows_num_wb_cwf_regs;

	/*metering*/
	int  rows_num_metering_matrix_regs;
	int  rows_num_metering_center_regs;
	int  rows_num_metering_spot_regs;

	/*EV*/
	int  rows_num_ev_minus_4_regs;
	int  rows_num_ev_minus_3_regs;
	int  rows_num_ev_minus_2_regs;
	int  rows_num_ev_minus_1_regs;
	int  rows_num_ev_default_regs;
	int  rows_num_ev_plus_1_regs;
	int  rows_num_ev_plus_2_regs;
	int  rows_num_ev_plus_3_regs;
	int  rows_num_ev_plus_4_regs;

	/*contrast*/
	int  rows_num_contrast_minus_2_regs;
	int  rows_num_contrast_minus_1_regs;
	int  rows_num_contrast_default_regs;
	int  rows_num_contrast_plus_1_regs;
	int  rows_num_contrast_plus_2_regs;

	/*sharpness*/
	int  rows_num_sharpness_minus_3_regs;
	int  rows_num_sharpness_minus_2_regs;
	int  rows_num_sharpness_minus_1_regs;
	int  rows_num_sharpness_default_regs;
	int  rows_num_sharpness_plus_1_regs;
	int  rows_num_sharpness_plus_2_regs;
	int  rows_num_sharpness_plus_3_regs;

	/*saturation*/
	int  rows_num_saturation_minus_2_regs;
	int  rows_num_saturation_minus_1_regs;
	int  rows_num_saturation_default_regs;
	int  rows_num_saturation_plus_1_regs;
	int  rows_num_saturation_plus_2_regs;

	/*zoom*/
	int  rows_num_zoom_00_regs;
	int  rows_num_zoom_01_regs;
	int  rows_num_zoom_02_regs;
	int  rows_num_zoom_03_regs;
	int  rows_num_zoom_04_regs;
	int  rows_num_zoom_05_regs;
	int  rows_num_zoom_06_regs;
	int  rows_num_zoom_07_regs;
	int  rows_num_zoom_08_regs;


	/*scene mode*/
	int  rows_num_scene_none_regs;
	int  rows_num_scene_portrait_regs;
	int  rows_num_scene_nightshot_regs;
	int  rows_num_scene_backlight_regs;
	int  rows_num_scene_landscape_regs;
	int  rows_num_scene_sports_regs;
	int  rows_num_scene_party_indoor_regs;
	int  rows_num_scene_beach_snow_regs;
	int  rows_num_scene_sunset_regs;
	int  rows_num_scene_duskdawn_regs;
	int  rows_num_scene_fall_color_regs;
	int  rows_num_scene_fireworks_regs;
	int  rows_num_scene_candle_light_regs;
	int  rows_num_scene_text_regs;

	/*fps*/
	int  rows_num_fps_auto_regs;
	int  rows_num_fps_5_regs;
	int  rows_num_fps_7_regs;
	int  rows_num_fps_10_regs;
	int  rows_num_fps_15_regs;
	int  rows_num_fps_20_regs;
	int  rows_num_fps_25_regs;
	int  rows_num_fps_30_regs;
	int  rows_num_fps_60_regs;
	int  rows_num_fps_120_regs;

	/*quality*/
	int  rows_num_quality_superfine_regs;
	int  rows_num_quality_fine_regs;
	int  rows_num_quality_normal_regs;
	int  rows_num_quality_economy_regs;

	/*preview size */
	int  rows_num_preview_size_176x144_regs;
	int  rows_num_preview_size_320x240_regs;
	int  rows_num_preview_size_352x288_regs;
	int  rows_num_preview_size_640x480_regs;
	int  rows_num_preview_size_704x576_regs;
	int  rows_num_preview_size_720x480_regs;
	int  rows_num_preview_size_800x480_regs;
	int  rows_num_preview_size_800x600_regs;
	int  rows_num_preview_size_1024x600_regs;
	int  rows_num_preview_size_1024x768_regs;
	int  rows_num_preview_size_1280x960_regs;
	int  rows_num_preview_size_1600x960_regs;
	int  rows_num_preview_size_1600x1200_regs;
	int  rows_num_preview_size_2048x1232_regs;
	int  rows_num_preview_size_2048x1536_regs;
	int  rows_num_preview_size_2560x1920_regs;

	/*Capture size */
	int  rows_num_capture_size_640x480_regs;
	int  rows_num_capture_size_720x480_regs;
	int  rows_num_capture_size_800x480_regs;
	int  rows_num_capture_size_800x486_regs;
	int  rows_num_capture_size_800x600_regs;
	int  rows_num_capture_size_1024x600_regs;
	int  rows_num_capture_size_1024x768_regs;
	int  rows_num_capture_size_1280x960_regs;
	int  rows_num_capture_size_1600x960_regs;
	int  rows_num_capture_size_1600x1200_regs;
	int  rows_num_capture_size_2048x1232_regs;
	int  rows_num_capture_size_2048x1536_regs;
	int  rows_num_capture_size_2560x1536_regs;
	int  rows_num_capture_size_2560x1920_regs;

	/*pattern*/
	int  rows_num_pattern_on_regs;
	int  rows_num_pattern_off_regs;

	/* AE */
	int  rows_num_ae_lock_regs;
	int  rows_num_ae_unlock_regs;

	/* AWB */
	int  rows_num_awb_lock_regs;
	int  rows_num_awb_unlock_regs;


};


#endif /* __CAMDRV_SS_H__ */
