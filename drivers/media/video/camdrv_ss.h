#ifndef __CAMDRV_SS_H__
#define __CAMDRV_SS_H__

#include <linux/i2c.h>
#include <linux/device.h>
#include <asm/gpio.h>
#include <mach/kona.h>
#include <mach/hawaii.h>
#include <linux/power_supply.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#include <linux/broadcom/bcm59055-power.h>
#include <linux/clk.h>
#include <linux/bootmem.h>
#include <plat/pi_mgr.h>
#include <linux/device.h>
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


#define CAM_ERROR_PRINTK(format, arg...)    printk(format, ## arg)

#define CAMDRV_SS_DEBUG

#ifdef CAMDRV_SS_DEBUG
#define CAM_INFO_PRINTK(format, arg...)    printk(format, ## arg)
#else
#define CAM_INFO_PRINTK(format, arg...)
#endif

#ifdef CONFIG_VIDEO_UNICAM_CAMERA
extern bool camdrv_ss_power(int cam_id , int bOn);
#endif


/**************************************************************************
* STRUCTURES
*************************************************************************/

/* Camera functional setting values configured by user concept */
struct camdrv_ss_userset {
	signed int exposure_bias;	/* V4L2_CID_EXPOSURE */
	unsigned int auto_wb;		/* V4L2_CID_AUTO_WHITE_BALANCE */
	unsigned int manual_wb;		/* V4L2_CID_WHITE_BALANCE_PRESET */
	unsigned int effect;		/* Color FX (AKA Color tone) */
	unsigned int contrast;		/* V4L2_CID_CONTRAST */
	unsigned int saturation;	/* V4L2_CID_SATURATION */
	unsigned int sharpness;		/* V4L2_CID_SHARPNESS */
};

struct camdrv_jpeg_param {
	unsigned int enable;
	unsigned int quality;
	unsigned int main_size;  /* Main JPEG file size */
	unsigned int thumb_size; /* Thumbnail file size */
	unsigned int main_offset;
	unsigned int thumb_offset;
	unsigned int postview_offset;
} ;

struct camdrv_ss_version {
	unsigned int major;
	unsigned int minor;
};

struct camdrv_ss_firmware {
	unsigned int addr;
	unsigned int size;
};

struct camdrv_ss_af_info {
	int x;
	int y;
	int preview_width;
	int preview_height;
};

struct camdrv_ss_image_info {
	int width;
	int height;
};

enum camdrv_ss_mode_switch {
	INIT_DONE_TO_CAMERA_PREVIEW = 0,		/* Initilization done : need to do camera preview */
	CAMERA_PREVIEW_SIZE_CHANGE,			/* cam_preview to cam_preview : size-resolution change in camera mode */
	CAMERA_PREVIEW_TO_PICTURE_CAPTURE,		/* capture mode */
	PICTURE_CAPTURE_TO_CAMERA_PREVIEW_RETURN,	/* back to preview: preview return */
	INIT_DONE_TO_CAMCORDER_PREVIEW,			/* Initialization done: need to do camcorder preview */
	CAMERA_PREVIEW_TO_CAMCORDER_PREVIEW,		/* camera to camcorder */
	CAMCORDER_PREVIEW_SIZE_CHANGE,			/* size -resolution change in camcorder mode */
	CAMCORDER_PREVIEW_TO_CAMERA_PREVIEW,		/* camcorder to camera */
};

enum camdrv_ss_capture_mode_state {
	CAMDRV_SS_CAPTURE_MODE_READ_PROCESSING = 0,
	CAMDRV_SS_CAPTURE_MODE_READY,
	CAMDRV_SS_CAPTURE_MODE_NOT_SUPPORT,
	CAMDRV_SS_CAPTURE_MODE_READ_FAILED
};

struct camdrv_ss_sensor_reg {
	bool isMainSensor;
	bool (*sensor_functions)(struct camdrv_ss_sensor_cap *sensor);
	int (*sensor_power)(int on);
	int (*read_device_id)(struct i2c_client *client, char *device_id);
	char name[50];
};

struct camdrv_ss_state {
	struct camera_platform_data *platform_data;
	struct v4l2_subdev sd;
	struct v4l2_pix_format pix;
	struct camdrv_ss_userset userset;
	struct camdrv_jpeg_param jpeg_param;
	struct v4l2_subdev_sensor_interface_parms *plat_parms;
	struct camdrv_ss_version fw_ver;
	struct camdrv_ss_af_info af_info;
	struct camdrv_ss_image_info postview_info;
	struct v4l2_streamparm strm;

	int capture_framesize_index;
	int preview_framesize_index;
	int camdrv_ss_version;
	int fps;

	int check_dataline;
	int current_flash_mode;
	int camera_flash_fire;
	int camera_af_flash_fire;
	int camera_af_flash_checked;

	int af_mode;
	int currentScene;
	int currentWB;
	int currentMetering;
	int currentEffect;
	int bStartFineSearch;
	v4l2_touch_area touch_area;
	bool bTouchFocus;
	enum CameraMode	current_mode;
	enum camdrv_ss_mode_switch  mode_switch;
};

struct camdrv_ss_framesize {
	u32 index;
	u32 width;
	u32 height;
};

int camdrv_ss_set_preview_size(struct v4l2_subdev *sd);
int camdrv_ss_set_dataline_onoff(struct v4l2_subdev *sd, int onoff);
int camdrv_ss_set_capture_size(struct v4l2_subdev *sd);

int camdrv_ss_i2c_write_2_bytes(struct i2c_client *client, unsigned char sub_addr, unsigned char data);
int camdrv_ss_i2c_read_1_byte(struct i2c_client *client, unsigned char subaddr, unsigned char *data);
int camdrv_ss_i2c_write_4_bytes(struct i2c_client *client, unsigned short subaddr, unsigned short data);
int camdrv_ss_i2c_read_2_bytes(struct i2c_client *client, unsigned short subaddr, unsigned short *data);

int camdrv_ss_i2c_set_config_register(struct i2c_client *client,
					regs_t reg_buffer[],
					int num_of_regs,
					char *name);


enum camdrv_ss_cam_light {
	CAM_LOW_LIGHT = 0,
	CAM_NORMAL_LIGHT,
	CAM_HIGH_LIGHT,
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
	int skip_frames;

	int delay_duration;


/*******************/
/* FUNCTION POINTERS */
/******************/

/* Drivers need to implement this mandatory */
	long (*thumbnail_ioctl)(struct v4l2_subdev *sd, unsigned int cmd, void *arg);
	int  (*enum_frameintervals)
		(struct v4l2_subdev *sd, struct v4l2_frmivalenum *fival);
	int (*get_exif_sensor_info)
		(struct v4l2_subdev *sd,
		struct v4l2_exif_sensor_info *exif_param);

/* Optional */
	int (*set_preview_start)(struct v4l2_subdev *sd);
	int (*set_capture_start)(struct v4l2_subdev *sd, struct v4l2_control *ctrl);
	int (*get_nightmode)(struct v4l2_subdev *sd); /* aska add */
	int (*set_iso)(struct v4l2_subdev *sd, int mode); /* aska add */
	int (*set_white_balance)(struct v4l2_subdev *sd, int mode); /* aska add */
	int (*get_ae_stable_status)(struct v4l2_subdev *sd, struct v4l2_control *ctrl);
	int (*set_auto_focus)(struct v4l2_subdev *sd, struct v4l2_control *ctrl);
	int (*get_auto_focus_status)(struct v4l2_subdev *sd, struct v4l2_control *ctrl);
	int  (*set_touch_focus_area)(struct v4l2_subdev *sd, enum v4l2_touch_af touch_af, v4l2_touch_area *touch_area);
	int  (*set_touch_focus)(struct v4l2_subdev *sd, enum v4l2_touch_af touch_af, v4l2_touch_area *touch_area);
	int  (*get_touch_focus_status)(struct v4l2_subdev *sd, struct v4l2_control *ctrl);
	int (*flash_control)(struct v4l2_subdev *sd, int control_mode);
	int (*i2c_set_data_burst)(struct i2c_client *client, regs_t reg_buffer[], int num_of_regs, char *name);
	bool (*check_flash_needed)(struct v4l2_subdev *sd);
	int (*get_light_condition)(struct v4l2_subdev *sd, int *Result);
	int (*sensor_power)(int on);
	bool (*getEsdStatus)(struct v4l2_subdev *sd);
	enum camdrv_ss_capture_mode_state
		(*get_mode_change_reg)(struct v4l2_subdev *sd);
	int (*set_scene_mode)(struct v4l2_subdev *sd, struct v4l2_control *ctrl);  /* denis */
	void (*smartStayChangeInitSetting)(struct camdrv_ss_sensor_cap *sensor);
        int(* get_prefalsh_on) (struct v4l2_subdev *sd, struct v4l2_control *ctrl); //Backporting Rhea to Hawaii: added to call sensor Specific preflash rotuine
    void (* rear_camera_vendorid) (char *);// add vendor id
/************************/
/* REGISTER TABLE SETTINGS */
/************************/
	const regs_t *init_regs;

	/*vt mode*/
	const regs_t *vt_mode_regs;

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
	const regs_t *single_af_start_regs;
	const regs_t *get_1st_af_search_status;
	const regs_t *get_2nd_af_search_status;
	const regs_t *single_af_stop_regs;
	const regs_t *focus_mode_off_regs;
	const regs_t *focus_mode_af_regs;
	const regs_t *focus_mode_macro_regs;
	const regs_t *focus_mode_facedetect_regs;
	const regs_t *focus_mode_infinity_regs;
	const regs_t *focus_mode_auto_regs;
	const regs_t *focus_mode_continuous_video_regs;
	const regs_t *focus_mode_continuous_picture_regs;
	const regs_t *focus_mode_continuous_picture_macro_regs;

	/*effect*/
	const regs_t *effect_normal_regs;
	const regs_t *effect_negative_regs;
	const regs_t *effect_sepia_regs;
	const regs_t *effect_mono_regs;
	const regs_t *effect_aqua_regs;
	const regs_t *effect_sharpen_regs;
	const regs_t *effect_solarization_regs;
	const regs_t *effect_black_white_regs;
	const regs_t *effect_emboss_regs;
	const regs_t *effect_outline_regs;

	/*wb*/
	const regs_t *wb_auto_regs;
	const regs_t *wb_sunny_regs;
	const regs_t *wb_cloudy_regs;
	const regs_t *wb_tungsten_regs;
	const regs_t *wb_fluorescent_regs;
	const regs_t *wb_cwf_regs;
	const regs_t *wb_daylight_regs;
	const regs_t *wb_incandescent_regs;
	const regs_t *wb_shade_regs;
	const regs_t *wb_horizon_regs;

	/*metering*/
	const regs_t *metering_matrix_regs;
	const regs_t *metering_center_regs;
	const regs_t *metering_spot_regs;

	/*EV*/
	const regs_t *ev_minus_4_regs;
	const regs_t *ev_minus_3_regs;
	const regs_t *ev_minus_2_regs;
	const regs_t *ev_minus_1_5_regs;
	const regs_t *ev_minus_1_regs;
	const regs_t *ev_minus_0_5_regs;
	const regs_t *ev_default_regs;
	const regs_t *ev_plus_0_5_regs;
	const regs_t *ev_plus_1_regs;
	const regs_t *ev_plus_1_5_regs;
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
	const regs_t *scene_nightshot_dark_regs; /* aska add */
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
	const regs_t *HD_Camcorder_regs;
	const regs_t *HD_Camcorder_Disable_regs;
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

	/* ISO*/
	const regs_t *iso_auto_regs;
	const regs_t *iso_50_regs;
	const regs_t *iso_100_regs;
	const regs_t *iso_200_regs;
	const regs_t *iso_400_regs;
	const regs_t *iso_800_regs;
	const regs_t *iso_1200_regs;
	const regs_t *iso_1600_regs;
	const regs_t *iso_2400_regs;
	const regs_t *iso_3200_regs;
	const regs_t *iso_sports_regs;
	const regs_t *iso_night_regs;
	const regs_t *iso_movie_regs;

	/* WDR */
	const regs_t *wdr_on_regs;
	const regs_t *wdr_off_regs;

	/* CCD EV */
	const regs_t *ev_camcorder_minus_4_regs;
	const regs_t *ev_camcorder_minus_3_regs;
	const regs_t *ev_camcorder_minus_2_regs;
	const regs_t *ev_camcorder_minus_1_regs;
	const regs_t *ev_camcorder_default_regs;
	const regs_t *ev_camcorder_plus_1_regs;
	const regs_t *ev_camcorder_plus_2_regs;
	const regs_t *ev_camcorder_plus_3_regs;
	const regs_t *ev_camcorder_plus_4_regs;

	/* auto contrast */
	const regs_t *auto_contrast_on_regs;
	const regs_t *auto_contrast_off_regs;

	/* af return & focus mode */
	const regs_t *af_return_inf_pos;
	const regs_t *af_return_macro_pos;
        const  regs_t *main_flash_off_regs; //Add for nevis
        const regs_t *Pre_Flash_Start_EVT1;
        const regs_t *Pre_Flash_End_EVT1;
        const regs_t *Main_Flash_Start_EVT1;
        const regs_t *Main_Flash_End_EVT1;
        const regs_t *focus_mode_auto_regs_cancel1;
        const regs_t *focus_mode_auto_regs_cancel2;
        const regs_t *focus_mode_auto_regs_cancel3;
        const regs_t *focus_mode_macro_regs_cancel1;
        const regs_t *focus_mode_macro_regs_cancel2;
        const regs_t *focus_mode_macro_regs_cancel3;

	/* flicker */
	const regs_t *antibanding_50hz_regs;
	const regs_t *antibanding_60hz_regs;
        
	/* NO OF ROWS OF EACH REGISTER SETTING */
	int  rows_num_init_regs;
	int  rows_num_vt_mode_regs;
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
	int  rows_num_single_af_start_regs;
	int  rows_num_get_1st_af_search_status;
	int  rows_num_get_2nd_af_search_status;
	int  rows_num_single_af_stop_regs;
	int  rows_num_focus_mode_off_regs;
	int  rows_num_focus_mode_af_regs;
	int  rows_num_focus_mode_macro_regs;
	int  rows_num_focus_mode_facedetect_regs;
	int  rows_num_focus_mode_infinity_regs;
	int  rows_num_focus_mode_auto_regs;
	int  rows_num_focus_mode_continuous_video_regs;
	int  rows_num_focus_mode_continuous_picture_regs;
	int  rows_num_focus_mode_continuous_picture_macro_regs;

	/*effect*/
	int  rows_num_effect_normal_regs;
	int  rows_num_effect_negative_regs;
	int  rows_num_effect_sepia_regs;
	int  rows_num_effect_mono_regs;
	int  rows_num_effect_aqua_regs;
	int  rows_num_effect_sharpen_regs;
	int  rows_num_effect_solarization_regs;
	int  rows_num_effect_black_white_regs;
	int  rows_num_effect_emboss_regs;
	int  rows_num_effect_outline_regs;

	/*wb*/
	int  rows_num_wb_auto_regs;
	int  rows_num_wb_sunny_regs;
	int  rows_num_wb_cloudy_regs;
	int  rows_num_wb_tungsten_regs;
	int  rows_num_wb_fluorescent_regs;
	int  rows_num_wb_cwf_regs;
	int  rows_num_wb_daylight_regs;
	int  rows_num_wb_incandescent_regs;
	int  rows_num_wb_shade_regs;
	int  rows_num_wb_horizon_regs;

	/*metering*/
	int  rows_num_metering_matrix_regs;
	int  rows_num_metering_center_regs;
	int  rows_num_metering_spot_regs;

	/*EV*/
	int  rows_num_ev_minus_4_regs;
	int  rows_num_ev_minus_3_regs;
	int  rows_num_ev_minus_2_regs;
	int  rows_num_ev_minus_1_5_regs;
	int  rows_num_ev_minus_1_regs;
	int  rows_num_ev_minus_0_5_regs;
	int  rows_num_ev_default_regs;
	int  rows_num_ev_plus_0_5_regs;
	int  rows_num_ev_plus_1_regs;
	int  rows_num_ev_plus_1_5_regs;
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
	int  rows_num_scene_nightshot_dark_regs; /* aska add */
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
	int  rows_num_HD_Camcorder_regs;
	int  rows_num_HD_Camcorder_Disable_regs;
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
	/* ISO*/
	int rows_num_iso_auto_regs;
	int rows_num_iso_50_regs;
	int rows_num_iso_100_regs;
	int rows_num_iso_200_regs;
	int rows_num_iso_400_regs;
	int rows_num_iso_800_regs;
	int rows_num_iso_1200_regs;
	int rows_num_iso_1600_regs;
	int rows_num_iso_2400_regs;
	int rows_num_iso_3200_regs;
	int rows_num_iso_sports_regs;
	int rows_num_iso_night_regs;
	int rows_num_iso_movie_regs;


	/* WDR */
	int rows_num_wdr_on_regs;
	int rows_num_wdr_off_regs;

	/* CCD EV */
	int rows_num_ev_camcorder_minus_4_regs;
	int rows_num_ev_camcorder_minus_3_regs;
	int rows_num_ev_camcorder_minus_2_regs;
	int rows_num_ev_camcorder_minus_1_regs;
	int rows_num_ev_camcorder_default_regs;
	int rows_num_ev_camcorder_plus_1_regs;
	int rows_num_ev_camcorder_plus_2_regs;
	int rows_num_ev_camcorder_plus_3_regs;
	int rows_num_ev_camcorder_plus_4_regs;


	/* auto contrast */
	int rows_num_auto_contrast_on_regs;
	int rows_num_auto_contrast_off_regs;


	/* af return & focus mode */
	int rows_num_af_return_inf_pos;
	int rows_num_af_return_macro_pos;
        int  rows_num_main_flash_off_regs; //Add for nevis

  int rows_num_Pre_Flash_Start_EVT1;
  int rows_num_Pre_Flash_End_EVT1;
  int rows_num_Main_Flash_Start_EVT1;
  int rows_num_Main_Flash_End_EVT1;
  int rows_num_focus_mode_auto_regs_cancel1;
  int rows_num_focus_mode_auto_regs_cancel2;
  int rows_num_focus_mode_auto_regs_cancel3;
  int rows_num_focus_mode_macro_regs_cancel1;
  int rows_num_focus_mode_macro_regs_cancel2;
  int rows_num_focus_mode_macro_regs_cancel3;

	/* flicker */
	int rows_num_antibanding_60hz_regs;        
	int rows_num_antibanding_50hz_regs;
};
/************************/
/* EXTERN */
/************************/
extern int camdrv_ss_sensors_register(struct camdrv_ss_sensor_reg *sens);


extern unsigned int HWREV;

#endif /* __CAMDRV_SS_H__ */
