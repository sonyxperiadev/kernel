/***********************************************************************
* Driver for S5K5CCGX (3MP Camera) from SAMSUNG SYSTEM LSI
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
***********************************************************************/
#include <uapi/linux/time.h>
#include <uapi/linux/i2c.h>
#include <media/v4l2-ctrls.h>
#include <uapi/linux/videodev2.h>
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
#include "mach/pinmux.h"
#include <mach/rdb/brcm_rdb_padctrlreg.h>
#include "camdrv_ss.h"
#include <linux/kthread.h>
#include <asm/atomic.h>
#include <linux/videodev2.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/log2.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <linux/videodev2_brcm.h>


/* #define CONFIG_LOAD_FILE */

#define CAMDRV_SS_MODULE_NAME_MAIN		"camdrv_ss"
#define CAMDRV_SS_MODULE_NAME_SUB		"camdrv_ss_sub"
#define FACTORY_CHECK
static int camera_id = 0;
int flash_check =0;
DECLARE_WAIT_QUEUE_HEAD(gCamdrvReadyQ);

enum camdrv_ss_sensor_init_state_t {
	CAMDRV_SS_NOT_INITIALIZED = 0,
	CAMDRV_SS_INITIALIZING,
	CAMDRV_SS_INITIALIZE_DONE,
	CAMDRV_SS_STREAMING,
	CAMDRV_SS_INITIALIZE_FAILED
};

atomic_t sensor_state = ATOMIC_INIT(CAMDRV_SS_NOT_INITIALIZED);
static struct pin_config GPIOSetup;

/* Camera capture mode change monitor */
#define CAP_MODE_CHANGE_MORNITOR_INTERVAL_MS	20
#define CAP_MODE_CHANGE_MORNITOR_TIMEOUT	15
static atomic_t gCapModeState = ATOMIC_INIT(CAMDRV_SS_CAPTURE_MODE_NOT_SUPPORT);

static int camdrv_ss_cap_mode_change_monitor(struct v4l2_subdev *sd);
static int camdrv_ss_cap_mode_change_monitor_thread_func(void *data);
/* Camera register settings */
static struct camdrv_ss_sensor_cap	sensor;
static int camdrv_ss_s_stream(struct v4l2_subdev *sd, int enable);
static int camdrv_ss_init_thread_func(void *data);
static int camdrv_ss_init(struct v4l2_subdev *sd, u32 val);
static int camdrv_ss_actual_sensor_power_up();
static void camdrv_ss_init_func(struct v4l2_subdev *sd);
static int camdrv_ss_set_preview_start(struct v4l2_subdev *sd);

static int (*camdrv_ss_generic_flash_control)
			(struct v4l2_subdev *sd, int control_mode);

static char camdrv_ss_main_name[50];
static char camdrv_ss_sub_name[50];

char camdrv_rear_camera_vendorid[10]; 
static struct camdrv_ss_sensor_reg sensors_registered[6];
static int sensors_reg_main_count;
static int sensors_reg_sub_count;
static int sensors_reg_total_count;
static bool (*camdrv_ss_sensor_init_main)(struct camdrv_ss_sensor_cap *sensor);
static bool (*camdrv_ss_sensor_init_sub)(struct camdrv_ss_sensor_cap *sensor);

bool camdrv_ss_power(int cam_id, int bOn);
static int camdrv_ss_subdev_power(struct v4l2_subdev *sd, int on);


#define CAMDRV_SS_CAM_ID_MAIN 0
#define CAMDRV_SS_CAM_ID_SUB 1


/**************************************************************************
* GLOBAL, STATIC VARIABLES
***************************************************************************/

/* protect s_ctrl calls */
static DEFINE_MUTEX(sensor_s_ctrl);

static int camdrv_ss_find_preview_framesize(u32 width, u32 height);
static int camdrv_ss_find_capture_framesize(u32 width, u32 height);



#ifdef FACTORY_CHECK
struct class *camera_class; /* /sys/class/camera */

struct device *sec_main_cam_dev = NULL; /* /sys/class/camera/rear/rear_type */
EXPORT_SYMBOL(sec_main_cam_dev);
struct device *sec_sub_cam_dev = NULL; /* /sys/class/camera/rear/rear_type */
EXPORT_SYMBOL(sec_sub_cam_dev);

struct device *rear_flash_dev;     /*sys/class/camera/rear*/
static dev_t rear_flash_devnum;

static bool cam_class_init = false;
ssize_t maincamtype_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	/* this is to display the ISP version name. Rhea is SOC type ,so sensor name will be displayed.*/
	CAM_INFO_PRINTK("%s  camdrv_ss_main_name = %s\n",
						__func__, camdrv_ss_main_name);
	return sprintf(buf, "%s\n", camdrv_ss_main_name);
}

ssize_t maincamtype_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	CAM_INFO_PRINTK("%s :  %s\n", __func__, buf);
	return size;
}

ssize_t maincamfw_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	CAM_INFO_PRINTK("%s  camdrv_ss_main_name = %s\n",
						__func__, camdrv_ss_main_name);
	//return sprintf(buf, "%s \n", camdrv_ss_main_name);
	return sprintf(buf, "%s %s\n", camdrv_ss_main_name,"N");
}

ssize_t maincamfw_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	CAM_INFO_PRINTK("%s :  %s\n", __func__, buf);
	return size;
}

ssize_t subcamtype_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	/* this is to display the ISP version name. Rhea is SOC type ,so sensor name will be displayed.*/

	CAM_INFO_PRINTK("%s  camdrv_ss_sub_name = %s\n",
						__func__, camdrv_ss_sub_name);
	return sprintf(buf, "%s\n", camdrv_ss_sub_name);
}

ssize_t subcamtype_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	CAM_INFO_PRINTK("%s :  %s\n", __func__, buf);

	return size;
}

ssize_t subcamfw_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	CAM_INFO_PRINTK("%s  camdrv_ss_sub_name = %s\n",
					__func__, camdrv_ss_sub_name);
	//return sprintf(buf, "%s \n", camdrv_ss_sub_name);
	  return sprintf(buf, "%s %s\n", camdrv_ss_sub_name,"N");
}

ssize_t subcamfw_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	CAM_INFO_PRINTK("%s :  %s\n", __func__, buf);
	return size;
}

static DEVICE_ATTR(rear_camtype, 0644, maincamtype_show, NULL);
static DEVICE_ATTR(rear_camfw, 0644, maincamfw_show, NULL);
static DEVICE_ATTR(front_camtype, 0644, subcamtype_show, NULL);
static DEVICE_ATTR(front_camfw, 0644, subcamfw_show, NULL);

int camera_antibanding = ANTI_BANDING_50HZ; /* default */
int camera_antibanding_get(void);

int camera_antibanding_get(void)
{
	return camera_antibanding;
}

ssize_t camera_antibanding_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count;

	count = sprintf(buf, "%d", camera_antibanding);
	CAM_INFO_PRINTK("%s : antibanding is %d\n", __func__, camera_antibanding);

	return count;
}

ssize_t camera_antibanding_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int tmp = 0;

	sscanf(buf, "%d", &tmp);
	if ((ANTI_BANDING_50HZ == tmp) || (ANTI_BANDING_60HZ == tmp)) {
		camera_antibanding = tmp;
		CAM_INFO_PRINTK("%s : antibanding is %d\n", __func__, camera_antibanding);
	}

	return size;
}

static struct device_attribute camera_antibanding_attr = {
	.attr = {
		.name = "anti-banding",
		.mode = (S_IRUSR|S_IRGRP | S_IWUSR|S_IWGRP)},
	.show = camera_antibanding_show,
	.store = camera_antibanding_store
};
// AT command flash implementation
/*Backporting Rhea to Hawaii start: */
ssize_t rear_flash_show(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	if (buf[0] == '0')
	{
		if (camdrv_ss_generic_flash_control != NULL)
			camdrv_ss_generic_flash_control
					(NULL, FLASH_CONTROL_OFF);
		CAM_INFO_PRINTK("[%s]: off!\n", __func__);
	}
	else
	{
	
		if (camdrv_ss_generic_flash_control != NULL)
			camdrv_ss_generic_flash_control
					(NULL, FLASH_CONTROL_MIDDLE_LEVEL);
		CAM_INFO_PRINTK("[%s]: on!\n", __func__);
	}

	return count;
}

ssize_t rear_flash_store(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{

	if (buf[0] == '0')
	{
		if (camdrv_ss_generic_flash_control != NULL) {
			camdrv_ss_generic_flash_control
				(NULL, FLASH_CONTROL_OFF);
			flash_check = 0;
		}
		CAM_INFO_PRINTK("[%s]: off!\n", __func__);
	}
	else
	{
		if (camdrv_ss_generic_flash_control != NULL) {
			camdrv_ss_generic_flash_control
				(NULL, FLASH_CONTROL_MIDDLE_LEVEL);
			flash_check = 1;
		}
		CAM_INFO_PRINTK("[%s]: on!\n", __func__);
	}

	return count;
}


static struct device_attribute dev_attr_rear_flash = {
	.attr = {
		.name = "rear_flash",
		.mode =(S_IRUSR|S_IRGRP | S_IWUSR|S_IWGRP)},     //( S_IWUSR|S_IWGRP|S_IROTH)},
		.show = rear_flash_show,
		.store = rear_flash_store
		
}; 
//Added changes to show vendor id in string *#34971539#

ssize_t camera_rear_vendorid_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count;
    CAM_INFO_PRINTK("%s : rear_vendorid is %d\n", __func__, camera_rear_vendorid_show);
	return sprintf(buf, "%s\n", camdrv_rear_camera_vendorid);
		
}
static DEVICE_ATTR(rear_vendorid, 0644, camera_rear_vendorid_show, NULL);

#endif
/*Backporting Rhea to Hawaii End*/ 

/**************************************************************************
* EXTERN VARIABLES
***************************************************************************/

/**************************************************************************
* FUNCTION DECLARE
***************************************************************************/


inline struct camdrv_ss_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct camdrv_ss_state, sd);
}

/**************************************************************************
* TUNING CONFIGURATION FUNCTIONS, DATAS
***************************************************************************/
#ifdef CONFIG_LOAD_FILE

#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define MAX_REG_TABLE_LEN 3500
#define MAX_ONE_LINE_LEN 500

typedef struct {
	char name[100];
	char *location_ptr;
} reg_hash_t;

typedef struct regs_array_type {
	unsigned short subaddr;
	unsigned short value;
} regs_short_t;


static char *regs_buf_ptr = NULL;
static char *curr_pos_ptr = NULL;
static char current_line[MAX_ONE_LINE_LEN];
static regs_short_t reg_table[MAX_REG_TABLE_LEN];
static int reg_num_of_element = 0;

/* Warning!! : Register order is very important in aspect of performance of loading regs. */
/* Place regs by the order as described in register header file. */
static reg_hash_t reg_hash_table[] = {
	{"init_regs",								NULL},
	{"preview_camera_regs",							NULL},
	{"snapshot_normal_regs",						NULL},
	{"snapshot_lowlight_regs",						NULL},
	{"snapshot_highlight_regs",						NULL},
	{"snapshot_nightmode_regs",						NULL},
	{"snapshot_flash_on_regs",						NULL},
	{"snapshot_af_preflash_on_regs",					NULL},
	{"snapshot_af_preflash_off_regs",					NULL},

	{"single_af_start_regs",						NULL},
	{"get_1st_af_search_status",						NULL},
	{"get_2nd_af_search_status",						NULL},
	{"single_af_stop_regs",							NULL},

	{"effect_normal_regs",							NULL},
	{"effect_negative_regs",						NULL},
	{"effect_sepia_regs",							NULL},
	{"effect_mono_regs",							NULL},
	{"effect_aqua_regs",							NULL},
	{"effect_sharpen_regs",							NULL},
	{"effect_solarization_regs",						NULL},
	{"effect_black_white_regs",						NULL},

	{"wb_auto_regs",							NULL},
	{"wb_daylight_regs",							NULL},
	{"wb_cloudy_regs",							NULL},
	{"wb_incandescent_regs",						NULL},
	{"wb_fluorescent_regs",							NULL},

	{"metering_matrix_regs",						NULL},
	{"metering_center_regs",						NULL},
	{"metering_spot_regs",							NULL},

	{"ev_minus_4_regs",							NULL},
	{"ev_minus_3_regs",							NULL},
	{"ev_minus_2_regs",							NULL},
	{"ev_minus_1_regs",							NULL},
	{"ev_default_regs",							NULL},
	{"ev_plus_1_regs",							NULL},
	{"ev_plus_2_regs",							NULL},
	{"ev_plus_3_regs",							NULL},
	{"ev_plus_4_regs",							NULL},

	{"contrast_minus_2_regs",						NULL},
	{"contrast_minus_1_regs",						NULL},
	{"contrast_default_regs",						NULL},
	{"contrast_plus_1_regs",						NULL},
	{"contrast_plus_2_regs",						NULL},

	{"sharpness_minus_3_regs",						NULL},
	{"sharpness_minus_2_regs",						NULL},
	{"sharpness_minus_1_regs",						NULL},
	{"sharpness_default_regs",						NULL},
	{"sharpness_plus_1_regs",						NULL},
	{"sharpness_plus_2_regs",						NULL},
	{"sharpness_plus_3_regs",						NULL},

	{"saturation_minus_2_regs",						NULL},
	{"saturation_minus_1_regs",						NULL},
	{"saturation_default_regs",						NULL},
	{"saturation_plus_1_regs",						NULL},
	{"saturation_plus_2_regs",						NULL},

	{"zoom_00_regs",							NULL},
	{"zoom_01_regs",							NULL},
	{"zoom_02_regs",							NULL},
	{"zoom_03_regs",							NULL},
	{"zoom_04_regs",							NULL},
	{"zoom_05_regs",							NULL},
	{"zoom_06_regs",							NULL},
	{"zoom_07_regs",							NULL},
	{"zoom_08_regs",							NULL},

	{"scene_none_regs",							NULL},
	{"scene_portrait_regs",							NULL},
	{"scene_nightshot_regs",						NULL},
	{"scene_backlight_regs",						NULL},
	{"scene_landscape_regs",						NULL},
	{"scene_sports_regs",							NULL},
	{"scene_party_indoor_regs",						NULL},
	{"scene_beach_snow_regs",						NULL},
	{"scene_sunset_regs",							NULL},
	{"scene_duskdawn_regs",							NULL},
	{"scene_fall_color_regs",						NULL},
	{"scene_fireworks_regs",						NULL},
	{"scene_candle_light_regs",						NULL},
	{"scene_text_regs",							NULL},

	{"fps_auto_regs",							NULL},
	{"fps_5_regs",								NULL},
	{"fps_7_regs",								NULL},
	{"fps_10_regs",								NULL},
	{"fps_15_regs",								NULL},
	{"fps_20_regs",								NULL},
	{"fps_25_regs",								NULL},
	{"fps_30_regs",								NULL},
	{"fps_60_regs",								NULL},
	{"fps_120_regs",							NULL},

	{"quality_superfine_regs",						NULL},
	{"quality_fine_regs",							NULL},
	{"quality_normal_regs",							NULL},
	{"quality_economy_regs",						NULL},

	{"preview_size_176x144_regs",						NULL},
	{"preview_size_320x240_regs",						NULL},
	{"preview_size_352x288_regs",						NULL},
	{"preview_size_640x480_regs",						NULL},
	{"preview_size_704x576_regs",						NULL},
	{"preview_size_720x480_regs",						NULL},
	{"preview_size_800x480_regs",						NULL},
	{"preview_size_800x600_regs",						NULL},
	{"preview_size_1024x600_regs",						NULL},
	{"preview_size_1024x768_regs",						NULL},
	{"HD_Camcorder_regs",						NULL},
	{"HD_Camcorder_Disable_regs",						NULL},
	{"preview_size_1280x960_regs",						NULL},
	{"preview_size_1600x960_regs",						NULL},
	{"preview_size_1600x1200_regs",						NULL},
	{"preview_size_2048x1232_regs",						NULL},
	{"preview_size_2048x1536_regs",						NULL},
	{"preview_size_2560x1920_regs",						NULL},

	{"capture_size_640x480_regs",						NULL},
	{"capture_size_720x480_regs",						NULL},
	{"capture_size_800x480_regs",						NULL},
	{"capture_size_800x486_regs",						NULL},
	{"capture_size_800x600_regs",						NULL},
	{"capture_size_1024x600_regs",						NULL},
	{"capture_size_1024x768_regs",						NULL},
	{"capture_size_1280x960_regs",						NULL},
	{"capture_size_1600x960_regs",						NULL},
	{"capture_size_1600x1200_regs",						NULL},
	{"capture_size_2048x1232_regs",						NULL},
	{"capture_size_2048x1536_regs",						NULL},
	{"capture_size_2560x1536_regs",						NULL},
	{"capture_size_2560x1920_regs",						NULL},

	{"pattern_on_regs",							NULL},
	{"pattern_off_regs",							NULL},

	{"ae_lock_regs",							NULL},
	{"ae_unlock_regs",							NULL},

	{"awb_lock_regs",							NULL},
	{"awb_unlock_regs",							NULL},
	{"iso_auto_regs",							NULL},
	{"iso_50_regs",								NULL},
	{"iso_100_regs",							NULL},
	{"iso_200_regs",							NULL},
	{"iso_400_regs",							NULL},
	{"wdr_on_regs",								NULL},
	{"wdr_off_regs",							NULL},

	{"ev_camcorder_minus_4_regs",						NULL},
	{"ev_camcorder_minus_3_regs",						NULL},
	{"ev_camcorder_minus_2_regs",						NULL},
	{"ev_camcorder_minus_1_regs",						NULL},
	{"ev_camcorder_default_regs",						NULL},
	{"ev_camcorder_plus_1_regs",						NULL},
	{"ev_camcorder_plus_2_regs",						NULL},
	{"ev_camcorder_plus_3_regs",						NULL},
	{"ev_camcorder_plus_4_regs",						NULL},

	{"auto_contrast_on_regs",						NULL},
	{"auto_contrast_off_regs",						NULL},

	{"af_return_inf_pos",							NULL},
	{"af_return_macro_pos",							NULL},

	{"focus_mode_auto_regs",						NULL},
	{"focus_mode_macro_regs",						NULL},
	{"wb_iso_auto_regs",							NULL},
	{"wb_manual_iso_auto_regs",						NULL},
	{"wb_auto_iso_manual_regs",						NULL},
	{"wb_manual_iso_manual_regs",						NULL},
#if 0
	{"focus_mode_off_regs",							NULL},
	{"focus_mode_af_regs",							NULL},
	{"focus_mode_macro_regs",						NULL},
	{"focus_mode_facedetect_regs",						NULL},
	{"focus_mode_infinity_regs",						NULL},
	{"focus_mode_continuous_video_regs",					NULL},
	{"focus_mode_continuous_picture_regs",					NULL},
	{"focus_mode_continuous_picture_macro_regs",				NULL},
#endif
	{"vt_mode_regs",							NULL},
	{"init_regs_smart_stay",						NULL},
	
	{"Pre_Flash_Start_EVT1",						NULL},
	{"Pre_Flash_End_EVT1",						NULL},
	{"Main_Flash_Start_EVT1",						NULL},
	{"Main_Flash_End_EVT1",						NULL},
	{"focus_mode_auto_regs_cancel1",						NULL},
	{"focus_mode_auto_regs_cancel2",						NULL},
	{"focus_mode_auto_regs_cancel3",						NULL},
	{"focus_mode_macro_regs_cancel1",						NULL},
	{"focus_mode_macro_regs_cancel2",						NULL},
	{"focus_mode_macro_regs_cancel3",						NULL},
	{"antibanding_50hz_regs",						NULL},
	{"antibanding_60hz_regs",						NULL}
	
};

static bool camdrv_ss_regs_get_line(char *line_buf)
{
	int i;
	char *r_n_ptr = NULL;

	memset(line_buf, 0, MAX_ONE_LINE_LEN);

	r_n_ptr = strstr(curr_pos_ptr, "\n");

	/* exists. */
	if (r_n_ptr) {
		for (i = 0; i < MAX_ONE_LINE_LEN; i++) {
			if (curr_pos_ptr + i == r_n_ptr) {
				curr_pos_ptr = r_n_ptr + 1;
				break;
			}
			line_buf[i] = curr_pos_ptr[i];
		}
	line_buf[i] = '\0';

	return true;
	} else { /* doesn't exist. */
		if (strlen(curr_pos_ptr) > 0) {
			strcpy(line_buf, curr_pos_ptr);
			return true;
		} else {
			return false;
		}
	}
}


static bool camdrv_ss_regs_trim(char *line_buf)
{
	int left_index;
	int buff_len;
	int i;

	buff_len = strlen(line_buf);
	left_index  = -1;

	if (buff_len == 0) {
		return false;
	}

	/* Find the first letter that is not a white space from left side */
	for (i = 0; i < buff_len; i++) {
		if ((line_buf[i] != ' ') && (line_buf[i] != '\t') && (line_buf[i] != '\n') && (line_buf[i] != '\r')) {
			left_index = i;
			break;
		}
	}

	if (left_index == -1) {
		return false;
	}

	/* Skip comments and empty line */
	if ((line_buf[left_index] == '\0') || ((line_buf[left_index] == '/') && (line_buf[left_index + 1] == '/'))) {
		return false;
	}

	if (left_index != 0) {
		strcpy(line_buf, line_buf + left_index);
	}

	return true;
}


static int camdrv_ss_regs_parse_table(void)
{
	char reg_buf[7], data_buf[7]; /* , data_temp_buf[4]; */
	unsigned char reg[10];
	int reg_index = 0;

	memset(reg, '\0', strlen(reg));
	memset(reg_buf, '\0', strlen(reg_buf));
	/* memset(data_temp_buf, '\0', strlen(data_temp_buf)); */

	while (camdrv_ss_regs_get_line(current_line)) {
		if (camdrv_ss_regs_trim(current_line) == false) {
			continue;
		}

		/* Check End line of a table. */
		if ((current_line[0] == '}') && (current_line[1] == ';')) {
			break;
		}


		if (sensor.register_size == 4) {
			/* Parsing a register format : 0x0000F000->reg=0x0000, data=0xF000 */
			if ((current_line[0] == '0') && (current_line[10] == ',')) {
				memcpy(reg, (const void *)&current_line[0], 10);
				memcpy(reg_buf, (const void *)&current_line[0], 6);
				reg_table[reg_index].subaddr = (unsigned short)simple_strtoul(reg_buf, NULL, 16);
				reg_table[reg_index].value = (unsigned int)simple_strtoul(reg, NULL, 16) & 0x0000FFFF;

				/* memset(reg_buf, '\0', strlen(reg_buf)); */
				/* memset(reg, '\0', strlen(reg)); */
				/* CAM_PRINTK("%s,reg_table[%d].subaddr = 0x%x\n",__func__, reg_index, reg_table[reg_index].subaddr ); */
				/* CAM_PRINTK("%s,reg_table[%d].value = 0x%x\n",__func__, reg_index, reg_table[reg_index].value ); */
				reg_index++;
			}
		} else if (sensor.register_size == 2) {
			/* Parsing a register format : 0x0000, */
			if ((current_line[0] == '0') && (current_line[6] == ',')) {
				memcpy(reg, (const void *)&current_line[0], 6);
				reg_table[reg_index].value = (unsigned int)simple_strtoul(reg, NULL, 16)&0x0000FFFF;
				memset(reg, '\0', strlen(reg));

				/* CAM_PRINTK("%s,reg_table[%d].value = 0x%x\n",__func__, reg_index, reg_table[reg_index].value ); */
				reg_index++;
			}
		}
	}

	return reg_index;
}


static int camdrv_ss_regs_table_write(struct i2c_client *client, char *name)
{
	bool bFound_table = false;
	int i, err = 0;

	reg_num_of_element = 0;

	for (i = 0; i < sizeof(reg_hash_table)/sizeof(reg_hash_t); i++) {
		if (strcmp(name, reg_hash_table[i].name) == 0) {
			bFound_table = true;

			curr_pos_ptr = reg_hash_table[i].location_ptr;
			break;
		}
	}

	CAM_ERROR_PRINTK("%s, name : %s, bFound_table : %d\n", __func__, name, bFound_table);
	if (bFound_table) {
		reg_num_of_element = camdrv_ss_regs_parse_table();
	CAM_ERROR_PRINTK("%s, reg_num_of_element : %d\n", __func__, reg_num_of_element);
	} else {
		CAM_ERROR_PRINTK("[%s: %d] %s reg_table doesn't exist\n", __FILE__, __LINE__, name);
		return -EIO;
	}


	/* unsigned short next_subaddr = 0; */
	unsigned short subaddr = 0, data_value = 0;
	/* static unsigned char pBurstData[2048]; */
	int index;
	/* struct i2c_msg msg = {client->addr, 0, 0, 0}; */
	/* memset(pBurstData, 0, sizeof(pBurstData)); */

	CAM_ERROR_PRINTK("%s : sensor.register_size = %d\n", __func__, sensor.register_size);
	if (sensor.register_size == 4) {
		for (i = 0; i < reg_num_of_element; i++) {

			/* ata_value = reg_table[i].value; */
			subaddr = reg_table[i].subaddr;
			CAM_ERROR_PRINTK("sensor reg_table = 0x%x\n", reg_table[i]);

			switch (subaddr) {
#if 0
			/* case START_BURST_MODE: */
			case 0x0F12:
			{
				/* Start Burst datas */

				if (index == 0) {
					pBurstData[index++] = subaddr >> 8;
					pBurstData[index++] = subaddr & 0xFF;
				}

				pBurstData[index++] = data_value >> 8;
				pBurstData[index++] = data_value & 0xFF;

				/* Get Next Address */
				if ((i+1) == reg_num_of_element) { /* The last code */
					next_subaddr = 0xFFFF; /* Dummy */
				} else {
					/* next_subaddr = reg_buffer[i+1]>>16; */
					next_subaddr = reg_table[i+1].subaddr;
				}

				/* CAM_ERROR_PRINTK("%s  :i2c transfer 0x%x  0x%x 0x%x\n",__func__,subaddr,data_value,next_subaddr); */

				/* If next subaddr is different from the current subaddr */
				/* In other words, if burst mode ends, write the all of the burst datas which were gathered until now */
				if (next_subaddr != subaddr) {
					msg.buf = pBurstData;
					msg.len = index;

					err = i2c_transfer(client->adapter, &msg, 1);
					if (err < 0) {
						CAM_ERROR_PRINTK("[%s: %d] i2c burst write fail\n", __FILE__, __LINE__);
						return -EIO;
					}

					/* Intialize and gather busrt datas again. */
					index = 0;
					memset(pBurstData, 0, sizeof(pBurstData));
				}

				break;
			}
#endif /* #if 0 */

			/* case DELAY_SEQ: */
			case 0xFFFF:
			{
				msleep(data_value);
				break;
			}

			case 0xFCFC:
			case 0x0028:
			case 0x002A:
			default:
			{
				/* err = camdrv_ss_i2c_write_4_bytes(client, subaddr, data_value); */
				err = camdrv_ss_i2c_write_4_bytes(client,  reg_table[i].subaddr, reg_table[i].value);

				if (err < 0) {
					CAM_ERROR_PRINTK(" %s :i2c transfer failed !\n", __func__);
					return -EIO;
				}
				break;
			}
			} /* end of switch */
		} /* end of for */

	} else if (sensor.register_size == 2) { /* if(sensor.register_size==4) */

		unsigned char subaddr8 = 0, data_value8 = 0;

		for (i = 0; i < reg_num_of_element; i++) {
			data_value = reg_table[i].value;

			subaddr8 = data_value >> 8;
			data_value8 = data_value & 0xff;

			if (subaddr8 == sensor.delay_duration) {
				CAM_INFO_PRINTK(" %s :delay : %d ms\n", __func__, data_value8*10);
				msleep(data_value8*10);
				break;
			} else {
				/* CAM_PRINTK(" %s : data : 0x%4x\n", __func__, data_value); */

				err = camdrv_ss_i2c_write_2_bytes(client, subaddr8, data_value8);
				if (err < 0) {
					CAM_ERROR_PRINTK(" %s :i2c transfer failed !\n", __func__);
					return -EIO;
				}
			}
		} /* end of for */
	}

	/* sensor.i2c_set_data_burst(client, reg_table, reg_num_of_element); */

	return err;
}


#define CAMDRV_SS_TUNING_FILE_PATH   "/mnt/extSdCard/camtuning/camdrv_ss_%s.h"
#define CAMDRV_SS_MAX_PATH		255

int camdrv_ss_regs_table_init(struct v4l2_subdev *sd)
{
	struct file *filp;
	char *dp;
	long l;
	loff_t pos;
	int ret, i, retry_cnt;
	mm_segment_t fs = get_fs();
	char *location_ptr = NULL;
	bool bFound_name;
	char TUNING_FILE_PATH[CAMDRV_SS_MAX_PATH+1] = {0x00, };

	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	memset(TUNING_FILE_PATH, 0x00, CAMDRV_SS_MAX_PATH+1);
	sprintf(TUNING_FILE_PATH, CAMDRV_SS_TUNING_FILE_PATH, sensor.name);

	CAM_INFO_PRINTK("%s %d, PATH = %s, sensor name = %s\n", __func__, __LINE__, TUNING_FILE_PATH, sensor.name);

	set_fs(get_ds());

	if (TUNING_FILE_PATH == NULL) {
		CAM_ERROR_PRINTK("TUNING FILE PATH is NULL!!! %s %d\n", __func__, __LINE__);
		return -EIO;
	}

	filp = filp_open(TUNING_FILE_PATH, O_RDONLY, 0);

	if (IS_ERR(filp)) {
		CAM_ERROR_PRINTK("file open error\n");
		return -EIO;
	}

	l = filp->f_path.dentry->d_inode->i_size;
	CAM_ERROR_PRINTK("%s file size = %ld\n", __func__, l);

	msleep(50);

	CAM_ERROR_PRINTK("%s %d\n", __func__, __LINE__);

	for (retry_cnt = 5; retry_cnt > 0; retry_cnt--) {
		dp = vmalloc(l);

		if (dp != NULL) {
			break;
		}

		msleep(50);
	}

	if (dp == NULL) {
		CAM_ERROR_PRINTK("Out of Memory\n");
		filp_close(filp, current->files);
		return -ENOMEM;
	}

	memset(dp, 0, l);

	pos = 0;
	ret = vfs_read(filp, (char __user *)dp, l, &pos);

	if (ret != l) {
		CAM_ERROR_PRINTK("Failed to read file ret = %d\n", ret);
		vfree(dp);
		filp_close(filp, current->files);
		return -EINVAL;
	}

	filp_close(filp, current->files);

	set_fs(fs);

	regs_buf_ptr = dp;

	*((regs_buf_ptr + l) - 1) = '\0';

	/* Make hash table to enhance speed. */
	curr_pos_ptr = regs_buf_ptr;
	location_ptr = curr_pos_ptr;

	for (i = 0; i < sizeof(reg_hash_table)/sizeof(reg_hash_t); i++) {
		reg_hash_table[i].location_ptr = NULL;
		bFound_name = false;

		CAM_INFO_PRINTK("denis :::  %s %d, count = %d\n", __func__, __LINE__, i);
	    CAM_INFO_PRINTK("denis :::  reg_hash_table[%d].name : %s, current_line: %s\n", i, reg_hash_table[i].name, current_line);

		while (camdrv_ss_regs_get_line(current_line)) {
			if (strstr(current_line, reg_hash_table[i].name) != NULL) {
				bFound_name = true;
				reg_hash_table[i].location_ptr = location_ptr;
				break;
			}

			location_ptr = curr_pos_ptr;
		}

		CAM_INFO_PRINTK("denis :::  bFound_name = %d\n", bFound_name);


		if (bFound_name == false) {
			if (i == 0) {
				CAM_ERROR_PRINTK("[%s : %d] ERROR! Couldn't find the reg name in hash table\n", __FILE__, __LINE__);
				return -EIO;
			} else {
				curr_pos_ptr = reg_hash_table[i-1].location_ptr;
			}
			location_ptr = curr_pos_ptr;

			CAM_ERROR_PRINTK("[%s : %d] ERROR! Couldn't find the reg name in hash table\n", __FILE__, __LINE__);
		}
	}

	CAM_INFO_PRINTK("camdrv_ss_reg_table_init Done!\n");

	return 0;
}


void camdrv_ss_regs_table_exit(void)
{
	CAM_INFO_PRINTK("%s start\n", __func__);

	if (regs_buf_ptr) {
		vfree(regs_buf_ptr);
		regs_buf_ptr = NULL;
	}

	CAM_INFO_PRINTK("%s done\n", __func__);
}
#endif /* CONFIG_LOAD_FILE */

static int camdrv_ss_s_ctrl(struct v4l2_ctrl *ctrl);
static int camdrv_ss_g_ctrl(struct v4l2_ctrl *ctrl);
static int camdrv_ss_try_ctrl(struct v4l2_ctrl *ctrl);

const struct v4l2_ctrl_ops camdrv_ss_ctrl_ops = {
	.g_volatile_ctrl = camdrv_ss_g_ctrl,
	.s_ctrl = camdrv_ss_s_ctrl,
	.try_ctrl = camdrv_ss_try_ctrl,
};


/**************************************************************************
* FUNCTIONS
***************************************************************************/

/**************************************************************************
 * camdrv_ss_i2c_set_config_register: Read (I2C) multiple bytes to the camera sensor
 * @client: pointer to i2c_client
 * @subaddr: data to be written
 * @*data: buffer where data is read
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
int camdrv_ss_i2c_read_1_byte(struct i2c_client *client,
				unsigned char subaddr,
				unsigned char *data)
{
	unsigned char buf[4];
	struct i2c_msg msg = {client->addr, 0, 1, buf};
	int err = 0;

	if (!client->adapter) {
		CAM_ERROR_PRINTK("%s %s : client->adapter = NULL!!!\n", sensor.name, __func__);
		return -EIO;
	}

	buf[0] = subaddr;

	err = i2c_transfer(client->adapter, &msg, 1);
	if (err < 0) {
		CAM_ERROR_PRINTK("%s %s :i2c transfer failed at address %d !\n", sensor.name, __func__, subaddr);
		return -EIO;
	}

	msg.flags = I2C_M_RD;
	msg.len = 1;

	err = i2c_transfer(client->adapter, &msg, 1);
	if (err < 0) {
		CAM_ERROR_PRINTK("%s %s :i2c transfer failed at address %d !\n", sensor.name, __func__, subaddr);
		return -EIO;
	}

	*data = buf[0];

	return 0;
}

int camdrv_ss_i2c_read_2_bytes(struct i2c_client *client,
				unsigned short subaddr,
				unsigned short *data)
{
	unsigned char buf[4];
	struct i2c_msg msg = {client->addr, 0, 2, buf};
	int err = 0;

	if (!client->adapter) {
		CAM_ERROR_PRINTK("%s %s : client->adapter = NULL!!!\n", sensor.name, __func__);
		return -EIO;
	}

	buf[0] = subaddr >> 8;
	buf[1] = subaddr & 0xff;

	err = i2c_transfer(client->adapter, &msg, 1);
	if (err < 0) {
		CAM_ERROR_PRINTK("%s %s :i2c transfer failed at address %d !\n", sensor.name, __func__, subaddr);
		return -EIO;
	}

	msg.flags = I2C_M_RD;
	msg.len = 2;

	err = i2c_transfer(client->adapter, &msg, 1);
	if (err < 0) {
		CAM_ERROR_PRINTK("%s %s :i2c transfer failed at address %d !\n", sensor.name, __func__, subaddr);
		return -EIO;
	}

	*data = ((buf[0] << 8) | buf[1]);

	return 0;
}


/**************************************************************************
 * camdrv_ss_i2c_write_4_bytes: Write (I2C) multiple bytes to the camera sensor
 * @client: pointer to i2c_client
 * @subaddr: register address
 * @data: data to be written
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
int camdrv_ss_i2c_write_4_bytes(struct i2c_client *client,
				unsigned short subaddr,
				unsigned short data)
{
	unsigned char buf[4];
	struct i2c_msg msg = {client->addr, 0, 4, buf};
	int err = 0;
	if (!client->adapter) {
		CAM_ERROR_PRINTK("%s %s : client->adapter = NULL !\n", sensor.name, __func__);
		return -EIO;
	}
	/* CAM_ERROR_PRINTK("%s  :i2c transfer1 0x%x   %x\n", __func__, subaddr, data); */

	buf[0] = subaddr >> 8;
	buf[1] = subaddr & 0xFF;
	buf[2] = data >> 8;
	buf[3] = data & 0xFF;


	err = i2c_transfer(client->adapter, &msg, 1);

	/* CAM_ERROR_PRINTK("%s  :i2c transfer2 0x%x  %x\n", __func__, subaddr, data); */

	if (err < 0) {
		CAM_ERROR_PRINTK("%s %s :i2c transfer failed at address %d !\n", sensor.name, __func__, subaddr);
		return -EIO;
	}

	return 0;
}


int camdrv_ss_i2c_write_2_bytes(struct i2c_client *client,
	unsigned char sub_addr,
	unsigned char data)
{
	unsigned char buf[2];
	struct i2c_msg msg = {client->addr, 0, 2, buf};
	int err = 0;

	if (!client->adapter) {
		CAM_ERROR_PRINTK("%s %s : client->adapter = NULL !\n", sensor.name, __func__);
		return -EIO;
	}

	buf[0] = sub_addr;
	buf[1] = data & 0xff;

	err = i2c_transfer(client->adapter, &msg, 1);
	/* CAM_ERROR_PRINTK("%s %s :i2c transfer 0x%x\n", sensor.name,__func__, data); */

	if (err < 0) {
		CAM_ERROR_PRINTK("%s %s :i2c transfer failed at address %d !\n", sensor.name, __func__, data);
		return -EIO;
	}

	return 0;
}


/**************************************************************************
 * camdrv_ss_i2c_set_config_register: Write (I2C) multiple bytes to the camera sensor
 * @client: pointer to i2c_client
 * @reg_buffer: buffer which includes all registers to be written.
 * @num_of_regs: number of registers to be written.
 * @name : This will be used for tuning.
 * Returns 0 on success, <0 on error
 ***************************************************************************/
int camdrv_ss_i2c_set_config_register(struct i2c_client *client,
					regs_t reg_buffer[],
					int num_of_regs,
					char *name)
{
	int err = 0;
	int i = 0, count = 0;
	unsigned short subaddr = 0, data_value = 0, data_value1 = 0;
	unsigned char subaddr8 = 0, data_value8 = 0;
#ifdef CONFIG_LOAD_FILE
	err = camdrv_ss_regs_table_write(client, name);
#else

	CAM_ERROR_PRINTK("%s : %s : srn : %s\n", sensor.name, __func__, name);

	if ((sensor.i2c_set_data_burst != NULL) && (strcmp(name, "init_regs") == 0)) {
		CAM_INFO_PRINTK("%s, %s :: Burst mode enable :: name : %s, reg size : %d\n", sensor.name, __func__, name, sensor.register_size);
		err = sensor.i2c_set_data_burst(client, reg_buffer, num_of_regs, name);
	} else {

		CAM_INFO_PRINTK("%s, %s :: Burst mode disable :: name : %s reg_size=%d reg num=%d\n", sensor.name, __func__, name, sensor.register_size, num_of_regs);

		for (i = 0; i < num_of_regs; i++) {
			if (sensor.register_size == 4) {
				subaddr = (reg_buffer[i] >> 16);
				data_value = reg_buffer[i];

				if (subaddr == sensor.delay_duration) {

					msleep(data_value);
					/* CAM_INFO_PRINTK("%s :: register 4 size :: delay : delay_duration = 0x%x, real delay : %d ms\n",
						__func__, sensor.delay_duration, data_value); */

				} else {
				err = camdrv_ss_i2c_write_4_bytes(client, subaddr, data_value);
				}
			} else if (sensor.register_size == 2) {
				if (count != num_of_regs) {
					/* CAM_INFO_PRINTK("i=%d reg=%x\n", i, reg_buffer[i]); */

					data_value1 = (reg_buffer[i] >> 16);
					data_value = reg_buffer[i];

					subaddr8 = data_value >> 8;
					data_value8 = data_value & 0xff;
					/* CAM_INFO_PRINTK(" reg value=0x%x %x\n",subaddr8,data_value8); */

					if (subaddr8 == sensor.delay_duration) {
						count++;
						msleep(data_value8*10);
						CAM_INFO_PRINTK("%s :: register 2 size :: delay : delay_duration = 0x%x, real delay : %d ms\n",
						__func__, sensor.delay_duration, data_value8*10);
					} else {
						count++;
						err = camdrv_ss_i2c_write_2_bytes(client, subaddr8, data_value8);
					}
					/* aska test */
					if (data_value1 != 0) {
						subaddr8 = data_value1 >> 8;
						data_value8 = data_value1 & 0xff;
						/* CAM_INFO_PRINTK(" reg value=0x%x %x\n",subaddr8,data_value8); */

						if (subaddr8 == sensor.delay_duration) {
							count++;
							msleep(data_value8*10);
							CAM_INFO_PRINTK("%s :: register 2 size :: delay : delay_duration = 0x%x, real delay : %d ms\n",
								__func__, sensor.delay_duration, data_value8*10);
						} else {
							count++;
							err = camdrv_ss_i2c_write_2_bytes(client, subaddr8, data_value8);
						}
					}
					/* CAM_INFO_PRINTK("reg count=%d\n",count); */
				}
			} else {
				CAM_ERROR_PRINTK("%s %s :register size is not 4 or 2 bytes = %d !\n", sensor.name, __func__, sensor.register_size);
				return -EIO;
			}

			if (err < 0) {
				CAM_ERROR_PRINTK("%s %s :i2c transfer failed !\n", sensor.name, __func__);
				return -EIO;
			}
		}
	}

#endif /* CONFIG_LOAD_FILE */

	return err;
}


static int camdrv_ss_set_vt_mode(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s %s :camdrv_ss_set_vt_mode E\n", sensor.name, __func__);

	switch (ctrl->val) {

	case CAM_VT_MODE_3G:
	{
		if (sensor.vt_mode_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : CAM_VT_MODE_3G not supported !!!\n", sensor.name, __func__);
				 err = -EIO;
		} else
			err =  camdrv_ss_i2c_set_config_register(client, sensor.vt_mode_regs, sensor.rows_num_vt_mode_regs, "vt_mode_regs");

			break;
	}

	case CAM_VT_MODE_VOIP:
	{
		if (sensor.vt_mode_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : CAM_VT_MODE_VOIP not supported !!!\n", sensor.name, __func__);
				 err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.vt_mode_regs, sensor.rows_num_vt_mode_regs, "vt_mode_regs");

			break;
	}
	case CAM_VT_MODE_SMART_STAY:
	{
		int  tmp_sensor_state = atomic_read(&sensor_state);
		bool bAlreadyStreaming = false;
		if (sensor.smartStayChangeInitSetting == NULL) {
			CAM_ERROR_PRINTK("%s %s : CAM_VT_MODE_SMART_STAY not supported !!!\n", sensor.name, __func__);
				 return -EIO;
		}

		CAM_INFO_PRINTK("%s: smartStayChangeInitSetting called ..\n", __func__);
		sensor.smartStayChangeInitSetting(&sensor);

		if (tmp_sensor_state == CAMDRV_SS_NOT_INITIALIZED) {
			CAM_ERROR_PRINTK("%s %s : CAMDRV_SS_NOT_INITIALIZED! Initialize again! Normally this case will not happen! !\n", sensor.name, __func__);
			if (0 != camdrv_ss_actual_sensor_power_up()) {
				CAM_ERROR_PRINTK("%s %s :"
					"camdrv_ss_actual_sensor_power_up"
					"FAILED. Return ERR !!!!\n",
					sensor.name, __func__);
				return -1;
			}
		} else if (tmp_sensor_state == CAMDRV_SS_INITIALIZE_FAILED) {
			CAM_ERROR_PRINTK("%s %s :  CAMDRV_SS_INITIALIZE_FAILED !\n", sensor.name, __func__);
			return -EFAULT;
		} else if (tmp_sensor_state == CAMDRV_SS_INITIALIZING) {
			CAM_ERROR_PRINTK("%s %s : Waiting for the camera initalize thread to finish initialization .........\n", sensor.name, __func__);
			if (0 == wait_event_interruptible_timeout(gCamdrvReadyQ,
				((atomic_read(&sensor_state) == CAMDRV_SS_INITIALIZE_DONE) || (atomic_read(&sensor_state) == CAMDRV_SS_INITIALIZE_FAILED)), msecs_to_jiffies(30000))) { /* aska modify for ivory */
				CAM_ERROR_PRINTK("%s %s : TIMEOUT 6 sec, waited for event CAMDRV_SS_INITIALIZE_DONE: state =%d !\n", sensor.name, __func__, atomic_read(&sensor_state));
				return -EFAULT;
			}
			if (atomic_read(&sensor_state) == CAMDRV_SS_INITIALIZE_FAILED) {
				CAM_ERROR_PRINTK("%s %s :  CAMDRV_SS_INITIALIZE_FAILED happened in init thread ! return ERROR !\n", sensor.name, __func__);
				return -EFAULT;
			}
		} else if (tmp_sensor_state == CAMDRV_SS_INITIALIZE_DONE) {
			CAM_INFO_PRINTK("%s %s : CAMDRV_SS_INITIALIZE_DONE. Now set smart stay\n", sensor.name, __func__);
		} else if (tmp_sensor_state == CAMDRV_SS_STREAMING) {
			bAlreadyStreaming = true;
			CAM_INFO_PRINTK("%s %s : CAMDRV_SS_STREAMING. Now set smart stay and preview size\n", sensor.name, __func__);
		} else {
			CAM_ERROR_PRINTK("%s %s : WRONG sensor state = %d ! ERROR! FAILED  .\n", sensor.name, __func__, tmp_sensor_state);
			return -EFAULT;
		}

		/* previous initiliazation is success. Now put smart stay settings */
		camdrv_ss_init_func(sd);
		if (atomic_read(&sensor_state) != CAMDRV_SS_INITIALIZE_DONE) {
			CAM_ERROR_PRINTK("%s %s : SmartStay Initialization FAILED !!.\n", sensor.name, __func__);
			return -EFAULT;
		}
		else
			CAM_INFO_PRINTK("%s %s : SmartStay Initialization success.\n", sensor.name, __func__);

		if(bAlreadyStreaming) {
			if (sensor.set_preview_start != NULL) {
				CAM_ERROR_PRINTK("%s %s :sensor has defined its set_preview_start\n", sensor.name, __func__);
				sensor.set_preview_start(sd);
			} else
				camdrv_ss_set_preview_start(sd);
		}
		break;
	}

	default:
	{
		if (sensor.vt_mode_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : CAM_VT_MODE_3G not supported !!!\n", sensor.name, __func__);
				 err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.vt_mode_regs, sensor.rows_num_vt_mode_regs, "vt_mode_regs");

		break;
	}
	} /* end of switch */
	CAM_INFO_PRINTK("%s %s :camdrv_ss_set_vt_mode X\n", sensor.name, __func__);

	return 0;
}


static int camdrv_ss_set_flash_mode(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{

	struct camdrv_ss_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s %s :camdrv_ss_set_flash_mode E\n", sensor.name, __func__);

	switch (ctrl->val) {
	case FLASH_MODE_ON:
	{
		state->current_flash_mode = FLASH_MODE_ON;
		break;
	}

	case FLASH_MODE_AUTO:
	{
		state->current_flash_mode = FLASH_MODE_AUTO;
		break;
	}

	case FLASH_MODE_TORCH_ON:
	{
		if (sensor.flash_control != NULL)
			sensor.flash_control(sd, FLASH_CONTROL_LOW_LEVEL);
		else
			CAM_ERROR_PRINTK("%s %s :flash_control is NULL!!!s\n", sensor.name, __func__);

		break;
	}

	case FLASH_MODE_TORCH_OFF:
	{
		if (sensor.flash_control != NULL)
			sensor.flash_control(sd, FLASH_CONTROL_OFF);
		else
			CAM_ERROR_PRINTK("%s %s :flash_control is NULL!!!s\n", sensor.name, __func__);
		break;
	}

	case  FLASH_MAIN_OFF:
	{
		if (state->camera_flash_fire) {
			if (sensor.main_flash_off_regs != 0) {
				err = camdrv_ss_i2c_set_config_register(client,
					sensor.main_flash_off_regs,
					sensor.rows_num_main_flash_off_regs,
					"Main_Flash_End_EVT1");
				if (err < 0) {
					CAM_ERROR_PRINTK("[%s: %d] ERROR! Setting main_flash_off_regs\n", __FILE__, __LINE__);
				}
			}
			if (sensor.flash_control != NULL)
				sensor.flash_control(sd, FLASH_CONTROL_OFF);
			else
				CAM_ERROR_PRINTK("%s %s :flash_control is NULL!!!s\n", sensor.name, __func__);
		}
		break;
	}

	case FLASH_MODE_OFF:
	default:
	{
		if (sensor.flash_control != NULL)
			sensor.flash_control(sd, FLASH_CONTROL_OFF);
		else
			CAM_ERROR_PRINTK("%s %s :flash_control is NULL!!!s\n", sensor.name, __func__);

		state->current_flash_mode = FLASH_MODE_OFF;
		break;
	}
	}

	return 0;
}


/**************************************************************************
 * camdrv_ss_set_frame_rate
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int camdrv_ss_set_frame_rate(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s %s :camdrv_ss_set_frame_rate  E  = %d\n", sensor.name, __func__, ctrl->val);

	switch (ctrl->val) {
	case FRAME_RATE_AUTO:
	{
		if (sensor.fps_auto_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : fps_auto_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err =  camdrv_ss_i2c_set_config_register(client, sensor.fps_auto_regs, sensor.rows_num_fps_auto_regs, "fps_auto_regs");
		break;
	}

	case FRAME_RATE_5:
	{
		if (sensor.fps_5_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : fps_5_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err =  camdrv_ss_i2c_set_config_register(client, sensor.fps_5_regs, sensor.rows_num_fps_5_regs, "fps_5_regs");
		break;
	}

	case FRAME_RATE_7:
	{
		if (sensor.fps_7_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : FRAME_RATE_7 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_7_regs, sensor.rows_num_fps_7_regs, "fps_7_regs");
		break;
	}

	case FRAME_RATE_10:
	{
		if (sensor.fps_10_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : FRAME_RATE_10 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_10_regs, sensor.rows_num_fps_10_regs, "fps_10_regs");
		break;
	}

	case FRAME_RATE_15:
	{
		if (sensor.fps_15_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : FRAME_RATE_15 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_15_regs, sensor.rows_num_fps_15_regs, "fps_15_regs");
		break;
	}

	case FRAME_RATE_20:
	{
		if (sensor.fps_20_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : fps_20_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_20_regs, sensor.rows_num_fps_20_regs, "fps_20_regs");
		break;
	}

	case FRAME_RATE_25:
#if 1/*0*/ /* fix me~!! currently sensor does not support 25 fps for recording : P120316-5148 */
	{
		if (sensor.fps_25_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : FRAME_RATE_25 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_25_regs, sensor.rows_num_fps_25_regs, "fps_25_regs");
		break;
	}
#endif
	case FRAME_RATE_30:
	{
		if (sensor.fps_30_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : FRAME_RATE_30 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_30_regs, sensor.rows_num_fps_30_regs, "fps_30_regs");
		break;
	}

	case FRAME_RATE_60:
	{
		if (sensor.fps_60_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : fps_60_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_60_regs, sensor.rows_num_fps_60_regs, "fps_60_regs");
		break;
	}

	case FRAME_RATE_120:
	{
		if (sensor.fps_120_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : FRAME_RATE_120 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_120_regs, sensor.rows_num_fps_120_regs, "fps_120_regs");
		break;
	}

	default:
	{
		CAM_ERROR_PRINTK("%s %s : FRAME_RATE_DEFAULT not supported !!!\n", sensor.name, __func__);
		err = -EIO;

		break;
	}
	}


	state->fps = ctrl->val;


	return err;
}

/**************************************************************************
 * camdrv_ss_set_preview_stop
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/

int camdrv_ss_set_dataline_onoff(struct v4l2_subdev *sd, int onoff)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	if (onoff) {
		/* data line on */
		CAM_INFO_PRINTK("%s %s :ON !!\n", sensor.name, __func__);

		if (sensor.pattern_on_regs == NULL)
			CAM_ERROR_PRINTK("%s %s : pattern_on_regs is NULL, please check if it is needed !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.pattern_on_regs, sensor.rows_num_pattern_on_regs, "pattern_on_regs");

		if (err < 0) {
			CAM_ERROR_PRINTK("%s %s : i2c failed !!\n", sensor.name, __func__);
			return -EIO;
		}
	} else {
		/* data line off */
		CAM_INFO_PRINTK("%s %s :OFF !!\n", sensor.name, __func__);

		if (sensor.pattern_off_regs == NULL)
			CAM_ERROR_PRINTK("%s %s : pattern_off_regs is NULL, please check if it is needed !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.pattern_off_regs, sensor.rows_num_pattern_off_regs, "pattern_off_regs");

		if (err < 0) {
			CAM_ERROR_PRINTK("%s %s : i2c failed !!\n", sensor.name, __func__);
			return -EIO;
		}

		state->check_dataline = CHK_DATALINE_OFF;
	}
	msleep(100);

	CAM_INFO_PRINTK("%s %s :done .\n", sensor.name, __func__);

	return err;
}

static int  camdrv_ss_set_preview_stop(struct v4l2_subdev *sd)
{
	/* struct camdrv_ss_state *state = to_state(sd); */
	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */

	struct v4l2_ctrl ctrl;
	int err;

	CAM_INFO_PRINTK("%s %s : camdrv_ss_set_preview_stop\n", sensor.name, __func__);

	ctrl.val = AUTO_FOCUS_OFF;

	if (sensor.set_auto_focus != NULL) {
		err = sensor.set_auto_focus(sd, &ctrl);
		if (err < 0) {
			CAM_ERROR_PRINTK("%s %s : set_auto_focus error\n", sensor.name, __func__);
			return -1;
		}
	}

	return 0;
}


/**************************************************************************
 * camdrv_ss_set_dzoom
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int camdrv_ss_set_dzoom(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s %s : camdrv_ss_set_dzoom = %d\n", sensor.name, __func__, ctrl->val);

	switch (ctrl->val) {
	case ZOOM_LEVEL_0:
	{
		if (sensor.zoom_00_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : ZOOM_LEVEL_0 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_00_regs, sensor.rows_num_zoom_00_regs, "zoom_00_regs");
		break;
	}

	case ZOOM_LEVEL_1:
	{
		if (sensor.zoom_01_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : ZOOM_LEVEL_1 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_01_regs, sensor.rows_num_zoom_01_regs, "zoom_01_regs");
		break;
	}

	case ZOOM_LEVEL_2:
	{
		if (sensor.zoom_02_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : ZOOM_LEVEL_2 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_02_regs, sensor.rows_num_zoom_02_regs, "zoom_02_regs");
		break;
	}

	case ZOOM_LEVEL_3:
	{
		if (sensor.zoom_03_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : ZOOM_LEVEL_3 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_03_regs, sensor.rows_num_zoom_03_regs, "zoom_03_regs");
		break;
	}

	case ZOOM_LEVEL_4:
	{
		if (sensor.zoom_04_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : ZOOM_LEVEL_4 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_04_regs, sensor.rows_num_zoom_04_regs, "zoom_04_regs");
		break;
	}

	case ZOOM_LEVEL_5:
	{
		if (sensor.zoom_05_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : ZOOM_LEVEL_5 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_05_regs, sensor.rows_num_zoom_05_regs, "zoom_05_regs");
		break;
	}

	case ZOOM_LEVEL_6:
	{
		if (sensor.zoom_06_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : ZOOM_LEVEL_6 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_06_regs, sensor.rows_num_zoom_06_regs, "zoom_06_regs");
		break;
	}

	case ZOOM_LEVEL_7:
	{
		if (sensor.zoom_07_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : ZOOM_LEVEL_7 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_07_regs, sensor.rows_num_zoom_07_regs, "zoom_07_regs");
		break;
	}

	case ZOOM_LEVEL_8:
	{
		if (sensor.zoom_08_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : ZOOM_LEVEL_8 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_08_regs, sensor.rows_num_zoom_08_regs, "zoom_08_regs");
		break;
	}

	default:
	{
		CAM_ERROR_PRINTK("%s %s : default not supported !!!\n", sensor.name, __func__);
		break;
	}
	}

	return err;
}


/**************************************************************************
 * camdrv_ss_set_preview_size
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
int camdrv_ss_set_preview_size(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0, index;

	index = state->preview_framesize_index;

	CAM_INFO_PRINTK("%s %s : index = %d\n", sensor.name, __func__, index);

	switch (index) {
	case PREVIEW_SIZE_QCIF:
	{
		if (sensor.preview_size_176x144_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_QCIF not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_176x144_regs, sensor.rows_num_preview_size_176x144_regs, "preview_size_176x144_regs");
		break;
	}

	case PREVIEW_SIZE_QVGA:
	{
		if (sensor.preview_size_320x240_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_QVGA not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_320x240_regs, sensor.rows_num_preview_size_320x240_regs, "preview_size_320x240_regs");
		break;
	}

	case PREVIEW_SIZE_CIF:
	{
		if (sensor.preview_size_352x288_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_CIF not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_352x288_regs, sensor.rows_num_preview_size_352x288_regs, "preview_size_352x288_regs");
		break;
	}

	case PREVIEW_SIZE_VGA:
	{
		if (sensor.preview_size_640x480_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_VGA not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_640x480_regs, sensor.rows_num_preview_size_640x480_regs, "preview_size_640x480_regs");
		break;
	}

	case PREVIEW_SIZE_4CIF:
	{
		if (sensor.preview_size_704x576_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_4CIF not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_704x576_regs, sensor.rows_num_preview_size_704x576_regs, "preview_size_704x576_regs");
		break;
	}

	case PREVIEW_SIZE_D1:
	{
		if (sensor.preview_size_720x480_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_D1 not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_720x480_regs, sensor.rows_num_preview_size_720x480_regs, "preview_size_720x480_regs");
		break;
	}

	case PREVIEW_SIZE_WVGA:
	{
		if (sensor.preview_size_800x480_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_WVGA not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_800x480_regs, sensor.rows_num_preview_size_800x480_regs, "preview_size_800x480_regs");
		break;
	}

	case PREVIEW_SIZE_SVGA:
	{
		if (sensor.preview_size_800x600_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_SVGA not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_800x600_regs, sensor.rows_num_preview_size_800x600_regs, "preview_size_800x600_regs");
		break;
	}

	case PREVIEW_SIZE_WSVGA:
	{
		if (sensor.preview_size_1024x600_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_WSVGA not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_1024x600_regs, sensor.rows_num_preview_size_1024x600_regs, "preview_size_1024x600_regs");
		break;
	}

	case PREVIEW_SIZE_XGA:
	{
		if (sensor.preview_size_1024x768_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_WSVGA not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_1024x768_regs, sensor.rows_num_preview_size_1024x768_regs, "preview_size_1024x768_regs");
		break;
	}

	case PREVIEW_SIZE_1MP:
	{
		if (sensor.preview_size_1280x960_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_1MP not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_1280x960_regs, sensor.rows_num_preview_size_1280x960_regs, "preview_size_1280x960_regs");
		break;
	}

	case PREVIEW_SIZE_W1MP:
	{
		if (sensor.preview_size_1600x960_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_W1MP not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_1600x960_regs, sensor.rows_num_preview_size_1600x960_regs, "preview_size_1600x960_regs");
		break;
	}

	case PREVIEW_SIZE_2MP:
	{
		if (sensor.preview_size_1600x1200_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_W1MP not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_1600x1200_regs, sensor.rows_num_preview_size_1600x1200_regs, "preview_size_1600x1200_regs");
		break;
	}

	case PREVIEW_SIZE_W2MP:
	{
		if (sensor.preview_size_2048x1232_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_W1MP not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_2048x1232_regs, sensor.rows_num_preview_size_2048x1232_regs, "preview_size_2048x1232_regs");
		break;
	}

	case PREVIEW_SIZE_3MP:
	{
		if (sensor.preview_size_2048x1536_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_W1MP not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_2048x1536_regs, sensor.rows_num_preview_size_2048x1536_regs, "preview_size_2048x1232_regs");
		break;
	}

	case PREVIEW_SIZE_5MP:
	{
		if (sensor.preview_size_2560x1920_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : PREVIEW_SIZE_W1MP not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_2560x1920_regs, sensor.rows_num_preview_size_2560x1920_regs, "preview_size_2560x1920_regs");
		break;
	}

	default:
	{
		CAM_ERROR_PRINTK("%s %s : default case not supported !!!\n", sensor.name, __func__);
		err = -EINVAL;
		break;
	}
	}

	return err;
}


static int camdrv_ss_set_preview_start(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s %s :\n", sensor.name, __func__);

	if (!state->pix.width || !state->pix.height) {
		CAM_ERROR_PRINTK("%s %s : width or height is NULL!!!\n", sensor.name, __func__);
		return -EINVAL;
	}

	if (state->mode_switch == PICTURE_CAPTURE_TO_CAMERA_PREVIEW_RETURN) {
		if (sensor.preview_camera_regs == 0)
			CAM_ERROR_PRINTK("%s %s : Returned after a capture, preview_camera_regs is NULL  !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_camera_regs, sensor.rows_num_preview_camera_regs, "preview_camera_regs");

		if (err < 0) {
			CAM_ERROR_PRINTK("%s %s : I2C preview_camera_regs IS FAILED\n", sensor.name, __func__);
			return -EIO;
		}
	}

	err = camdrv_ss_set_preview_size(sd);
	if (err < 0) {
		CAM_ERROR_PRINTK("%s %s : camdrv_ss_set_preview_size is FAILED !!\n", sensor.name, __func__);
		return -EIO;
	}

	if (state->mode_switch == CAMERA_PREVIEW_TO_CAMCORDER_PREVIEW || state->mode_switch == INIT_DONE_TO_CAMCORDER_PREVIEW) {
		/* Fixed FPS */
		if (sensor.fps_30_regs != 0)
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_30_regs, sensor.rows_num_fps_30_regs, "fps_30_regs");
		else if (sensor.fps_25_regs != 0)
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_25_regs, sensor.rows_num_fps_25_regs, "fps_25_regs");
		else if (sensor.fps_20_regs != 0)
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_20_regs, sensor.rows_num_fps_20_regs, "fps_20_regs");
		else if (sensor.fps_15_regs != 0)
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_15_regs, sensor.rows_num_fps_15_regs, "fps_15_regs");
		else
			CAM_ERROR_PRINTK("%s %s : Fixed FPS setting is not supported for 30,25,20,15 fps !!\n", sensor.name, __func__);

		if (err < 0) {
			CAM_ERROR_PRINTK("%s %s : Fixed FPS setting is FAILED !!\n", sensor.name, __func__);
			return -EIO;
		}
	}

	state->camera_flash_fire = 0;
	state->camera_af_flash_checked = 0;

	if (state->check_dataline) { /* Output Test Pattern */
		err = camdrv_ss_set_dataline_onoff(sd, 1);
		if (err < 0) {
			CAM_ERROR_PRINTK("%s %s : check_dataline is FAILED !!\n", sensor.name, __func__);
			return -EIO;
		}
	}

	return 0;
}


static int camdrv_ss_set_jpeg_quality(struct v4l2_subdev *sd)
{
	struct camdrv_ss_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	int err = 0;
	int quality;
	int x ;

	quality = state->jpeg_param.quality;

	CAM_INFO_PRINTK("%s %s :  quality =%d\n", sensor.name, __func__, quality);
	x = 100/JPEG_QUALITY_MAX;
	quality = quality / x;
	if (quality >= JPEG_QUALITY_MAX)
		quality = JPEG_QUALITY_SUPERFINE;

	switch (quality) {
	case JPEG_QUALITY_SUPERFINE:
	{
		if (sensor.quality_superfine_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : JPEG_QUALITY_SUPERFINE not supported ,trying  below quality!!!\n", sensor.name, __func__);
		} else {
			err = camdrv_ss_i2c_set_config_register(client, sensor.quality_superfine_regs, sensor.rows_num_quality_superfine_regs, "quality_superfine_regs");
			break;
		}
	}

	case JPEG_QUALITY_FINE:
	{
		if (sensor.quality_fine_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : JPEG_QUALITY_FINE not supported , trying  below quality !!!\n", sensor.name, __func__);
		} else {
			err = camdrv_ss_i2c_set_config_register(client, sensor.quality_fine_regs, sensor.rows_num_quality_fine_regs, "quality_fine_regs");
			break;
		}
	}

	case JPEG_QUALITY_NORMAL:
	{
		if (sensor.quality_normal_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : JPEG_QUALITY_NORMAL not supported , trying  below quality!!\n", sensor.name, __func__);
		} else {
			err = camdrv_ss_i2c_set_config_register(client, sensor.quality_normal_regs, sensor.rows_num_quality_normal_regs, "quality_normal_regs");
			break;
		}
	}

	case JPEG_QUALITY_ECONOMY:
	{
		if (sensor.quality_economy_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : JPEG_QUALITY_ECONOMY not supported , trying  below quality!!!\n", sensor.name, __func__);
		} else {
			err = camdrv_ss_i2c_set_config_register(client, sensor.quality_economy_regs, sensor.rows_num_quality_economy_regs, "quality_economy_regs");
			break;
		}
	}

	default:
	{
		CAM_ERROR_PRINTK("%s %s : no level is  supported !!!\n", sensor.name, __func__);
		err = -EINVAL;
		break;
	}
	}

	return err;
}


int camdrv_ss_set_capture_size(struct v4l2_subdev *sd)
{
	struct camdrv_ss_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0, index;

	index = state->capture_framesize_index;

	CAM_INFO_PRINTK("%s Entered : capture_framesize_index = %d\n", __func__, index);

	if (index < 0) {
		CAM_ERROR_PRINTK("%s: Sensor didn't define capture settings for this resolution. So taking from preview settings.\n", __func__);
		return camdrv_ss_set_preview_size(sd);
	}

	switch (index) {
	/* ======================= 1.333 Ratio ======================================= */
	case CAPTURE_SIZE_VGA: /* 416x320 */
	{
		if (sensor.capture_size_640x480_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : capture_size_640x480_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_640x480_regs, sensor.rows_num_capture_size_640x480_regs, "capture_size_640x480_regs");

		state->postview_info.width = 640;
		state->postview_info.height = 480;
		break;
	}

	case CAPTURE_SIZE_WVGA:
	{
		if (sensor.capture_size_800x480_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : capture_size_800x480_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_800x480_regs, sensor.rows_num_capture_size_800x480_regs, "capture_size_800x480_regs");

		state->postview_info.width = 800;
		state->postview_info.height = 480;
		break;
	}

	case CAPTURE_SIZE_SVGA:	/* 800x600 */
	{
		if (sensor.capture_size_800x600_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : capture_size_800x600_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_800x600_regs, sensor.rows_num_capture_size_800x600_regs, "capture_size_800x600_regs");

		state->postview_info.width = 800;
		state->postview_info.height = 600;
		break;
	}

	case CAPTURE_SIZE_D1:		/* 720x480 */
	{
		if (sensor.capture_size_720x480_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : capture_size_720x480_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_720x480_regs, sensor.rows_num_capture_size_720x480_regs, "capture_size_720x480_regs");

		state->postview_info.width = 720;
		state->postview_info.height = 480;
		break;
	}

	case CAPTURE_SIZE_WSVGA:	/* 1024x600 */
	{
		if (sensor.capture_size_1024x600_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : capture_size_1024x600_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_1024x600_regs, sensor.rows_num_capture_size_1024x600_regs, "capture_size_1024x600_regs");

		state->postview_info.width = 1024;
		state->postview_info.height = 600;
		break;
	}

	case CAPTURE_SIZE_XGA:	/* 1024x768 */
	{
		if (sensor.capture_size_1024x768_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : capture_size_1024x768_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_1024x768_regs, sensor.rows_num_capture_size_1024x768_regs, "capture_size_1024x768_regs");

		state->postview_info.width = 1024;
		state->postview_info.height = 768;
		break;
	}

	case CAPTURE_SIZE_1MP:		/* 1280x960 */
	{
		if (sensor.capture_size_1280x960_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : capture_size_1280x960_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_1280x960_regs, sensor.rows_num_capture_size_1280x960_regs, "capture_size_1280x960_regs");

		state->postview_info.width = 1024;
		state->postview_info.height = 960;
		break;
	}

	case CAPTURE_SIZE_W1MP:		/* 1600x960 */
	{
		if (sensor.capture_size_1600x960_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : capture_size_1600x960_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_1600x960_regs, sensor.rows_num_capture_size_1600x960_regs, "capture_size_1600x960_regs");

		state->postview_info.width = 1600;
		state->postview_info.height = 960;
		break;
	}

	case CAPTURE_SIZE_2MP:	/* UXGA  - 1600x1200 */
	{
		if (sensor.capture_size_1600x1200_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : capture_size_1600x1200_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_1600x1200_regs, sensor.rows_num_capture_size_1600x1200_regs, "capture_size_1600x1200_regs");

		state->postview_info.width = 1600;
		state->postview_info.height = 1200;
		break;
	}

	case CAPTURE_SIZE_W2MP:		/* 2048x1232, 2.4MP */
	{
		if (sensor.capture_size_2048x1232_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : capture_size_2048x1232_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_2048x1232_regs, sensor.rows_num_capture_size_2048x1232_regs, "capture_size_2048x1232_regs");

		state->postview_info.width = 2048;
		/* state->postview_info.height = 1232; */ /* aska modify to  match UI */
		state->postview_info.height = 1152;
		break;
	}

	case CAPTURE_SIZE_3MP:	/*2048x1536*/
	{
		if (sensor.capture_size_2048x1536_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : capture_size_2048x1536_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_2048x1536_regs, sensor.rows_num_capture_size_2048x1536_regs, "capture_size_2048x1536_regs");

		state->postview_info.width = 2048;
		state->postview_info.height = 1536;
		break;
	}

	case CAPTURE_SIZE_5MP:		/* 2560x1920 */
	{
		if (sensor.capture_size_2560x1920_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : capture_size_2560x1920_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_2560x1920_regs, sensor.rows_num_capture_size_2560x1920_regs, "capture_size_2560x1920_regs");

		state->postview_info.width = 2560;
		state->postview_info.height = 1920;
		break;
	}

	default:
	{
		CAM_ERROR_PRINTK("%s %s : default case not supported !!!\n", sensor.name, __func__);
		err = -EINVAL;
		break;
	}
	}

	return err;
}

static int camdrv_ss_set_capture_start(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;
	int light_state = CAM_NORMAL_LIGHT;

	CAM_INFO_PRINTK("%s Entered\n", __func__);


	/* SENSOR DOESN"T SUPPORT AUTO FRAME RATE DURING PICTURE CAPTURE MODE. CHANGE THE FPS */
	/* CURRENTLY I SET THE ORDER 15,10,7,5,20,25,30 . PLEASE FEEL FREE TO MODIFY THE ORDER IF NECESSARY */
#if 0 /* aska modify */
	if (state->fps == FRAME_RATE_AUTO) {
		if (sensor.fps_15_regs) {
			CAM_ERROR_PRINTK("%s FRAME_RATE_AUTO not supported during pictureMode. So change to 15fps !!!\n", __func__);
			err =  camdrv_ss_i2c_set_config_register(client, sensor.fps_15_regs, sensor.rows_num_fps_15_regs, "fps_15_regs");
		} else if (sensor.fps_10_regs) {
			CAM_ERROR_PRINTK("%s FRAME_RATE_AUTO not supported during pictureMode. So change to 10fps !!!\n", __func__);
			err =  camdrv_ss_i2c_set_config_register(client, sensor.fps_10_regs, sensor.rows_num_fps_10_regs, "fps_10_regs");
		} else if (sensor.fps_7_regs) {
			CAM_ERROR_PRINTK("%s FRAME_RATE_AUTO not supported during pictureMode. So change to 7fps !!!\n", __func__);
			err =  camdrv_ss_i2c_set_config_register(client, sensor.fps_7_regs, sensor.rows_num_fps_7_regs, "fps_7_regs");
		} else if (sensor.fps_5_regs) {
			CAM_ERROR_PRINTK("%s FRAME_RATE_AUTO not supported during pictureMode. So change to 5fps !!!\n", __func__);
			err =  camdrv_ss_i2c_set_config_register(client, sensor.fps_5_regs, sensor.rows_num_fps_5_regs, "fps_5_regs");
		} else if (sensor.fps_20_regs) {
			CAM_ERROR_PRINTK("%s FRAME_RATE_AUTO not supported during pictureMode. So change to 20fps !!!\n", __func__);
			err =  camdrv_ss_i2c_set_config_register(client, sensor.fps_20_regs, sensor.rows_num_fps_20_regs, "fps_20_regs");
		} else if (sensor.fps_25_regs) {
			CAM_ERROR_PRINTK("%s FRAME_RATE_AUTO not supported during pictureMode. So change to 25fps !!!\n", __func__);
			err =  camdrv_ss_i2c_set_config_register(client, sensor.fps_25_regs, sensor.rows_num_fps_25_regs, "fps_25_regs");
		} else if (sensor.fps_30_regs) {
			CAM_ERROR_PRINTK("%s FRAME_RATE_AUTO not supported during pictureMode. So change to 30fps !!!\n", __func__);
			err =  camdrv_ss_i2c_set_config_register(client, sensor.fps_30_regs, sensor.rows_num_fps_30_regs, "fps_30_regs");
		} else
			CAM_ERROR_PRINTK("%s FRAME_RATE_AUTO not supported during pictureMode. NO OTHER FPS SUPPORTED !!!\n", __func__);

		if (err < 0)
			CAM_ERROR_PRINTK("%s FRAME RATE SETTING ERROR! FAILED !!!!\n", __func__);
		else
			CAM_INFO_PRINTK("%s frame rate setting success\n", __func__);
	}
#endif



	/* Set image size */
	err = camdrv_ss_set_capture_size(sd);
	if (err < 0) {
		CAM_ERROR_PRINTK("%s %s : camdrv_ss_set_capture_size not supported !!!\n", sensor.name, __func__);
		return -EIO;
	}

	if (state->currentScene == SCENE_MODE_NIGHTSHOT || state->currentScene == SCENE_MODE_FIREWORKS) {
		/* Set Snapshot registers */
		if (sensor.snapshot_nightmode_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : snapshot_nightmode_regs not supported !!!\n", sensor.name, __func__);
			err = -EIO;
		}

		err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_nightmode_regs, sensor.rows_num_snapshot_nightmode_regs, "snapshot_nightmode_regs");
		if (err < 0) {
			CAM_ERROR_PRINTK("[%s : %d] ERROR! Could not take a picture\n", __FILE__, __LINE__);
			return -EIO;
		}
	} else {
		/* Support 3rd Party App without AF Pre-Flash. */
		if (state->camera_af_flash_checked == 0) {
			state->camera_flash_fire = 0;

			if (state->current_flash_mode == FLASH_MODE_ON) {
				state->camera_flash_fire = 1;
			} else if (state->current_flash_mode == FLASH_MODE_AUTO) {
				bool bflash_needed = false;
				if (sensor.check_flash_needed)
					bflash_needed = sensor.check_flash_needed(sd);
				else
					CAM_ERROR_PRINTK("%s %s : check_flash_needed is NULL !!!\n", sensor.name, __func__);

				if (bflash_needed) {
					state->camera_flash_fire = 1;
				}
			}
		}

		if (state->camera_flash_fire) {
			/* Set Snapshot registers */
			if (sensor.snapshot_flash_on_regs == 0) {
				CAM_ERROR_PRINTK("%s %s : snapshot_lowlight_regs not supported !!!\n", sensor.name, __func__);
				err = -EIO;
			}
			err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_flash_on_regs, sensor.rows_num_snapshot_flash_on_regs,
							"snapshot_flash_on_regs");
			if (err < 0) {
				CAM_ERROR_PRINTK("[%s : %d] ERROR! Couldn't Set Flash_on_regs\n", __FILE__, __LINE__);
			}
			if (sensor.flash_control == NULL)
				CAM_ERROR_PRINTK("%s %s : flash_control not supported !!!\n", sensor.name, __func__);
			else
				sensor.flash_control(sd, FLASH_CONTROL_MAX_LEVEL);
		}

		if (sensor.get_light_condition != NULL) {
			sensor.get_light_condition(sd, &light_state);
			CAM_ERROR_PRINTK("%s %s : light_state =%d !!\n", sensor.name, __func__, light_state);
		}

		if (light_state == CAM_LOW_LIGHT) {
			if (sensor.snapshot_lowlight_regs == 0) {
				CAM_ERROR_PRINTK("%s %s : snapshot_lowlight_regs not supported !!!\n", sensor.name, __func__);
				err = -EIO;
			}

			err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_lowlight_regs, sensor.rows_num_snapshot_lowlight_regs, "snapshot_highlight_regs");
			if (err < 0) {
				CAM_ERROR_PRINTK("%s %s : camdrv_ss_i2c_set_config_register failed  not supported !!!\n", sensor.name, __func__);
				return -EIO;
			}
		} else if (light_state == CAM_HIGH_LIGHT) {
			/* Set Snapshot registers */
			if (sensor.snapshot_highlight_regs == 0) {
				CAM_ERROR_PRINTK("%s %s : snapshot_highlight_regs not supported !!!\n", sensor.name, __func__);
				err = -EIO;
			}

			err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_highlight_regs, sensor.rows_num_snapshot_highlight_regs, "snapshot_highlight_regs");
			if (err < 0) {
				CAM_ERROR_PRINTK("%s %s : camdrv_ss_i2c_set_config_register failed  not supported !!!\n", sensor.name, __func__);
				return -EIO;
			}
		}

	}


	/* Set Snapshot registers */
	if (sensor.snapshot_normal_regs == 0) {
		CAM_ERROR_PRINTK("%s %s : snapshot_normal_regs not supported !!!\n", sensor.name, __func__);
		err = -EIO;
	}

	err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_normal_regs, sensor.rows_num_snapshot_normal_regs, "snapshot_normal_regs");
	if (err < 0) {
		CAM_ERROR_PRINTK("%s %s : camdrv_ss_i2c_set_config_register failed  not supported !!!\n", sensor.name, __func__);
		return -EIO;
	}

	return 0;
}

static int camdrv_ss_set_capture_done(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s\n", __func__);
     /*Backporting Rhea to Hawaii start:Check if preflash was fired then only apply  settings*/
	if (state->camera_af_flash_fire) {
		if (sensor.snapshot_af_preflash_off_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : snapshot_af_preflash_off_regs not supported !!!\n", sensor.name, __func__);
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_af_preflash_off_regs, sensor.rows_num_snapshot_af_preflash_off_regs, "snapshot_af_preflash_off_regs");

		if (err < 0) {
			CAM_ERROR_PRINTK("[%s: %d] ERROR! Setting af_preflash_off_regs\n", __FILE__, __LINE__);
		}
	}

       if (state->camera_flash_fire) /* Backporting Rhea to Hawaii:Check if flash is on then only apply main flash off settings*/
       {
	if (sensor.flash_control != NULL) {
		sensor.flash_control(sd, FLASH_CONTROL_OFF); /*Backporting Rhea to Hawaii:Turn off the main flash after capture done*/
               //Add main flash off setting for Nevis
               if(sensor.main_flash_off_regs != 0) 
               {
		     err = camdrv_ss_i2c_set_config_register(client, sensor.main_flash_off_regs, sensor.rows_num_main_flash_off_regs, "Main_Flash_End_EVT1");
		     if (err < 0) {
			CAM_ERROR_PRINTK("[%s: %d] ERROR! Setting main_flash_off_regs\n", __FILE__, __LINE__);
		    }
               }
        }
         //   state->camera_flash_fire = 0;

	   }
/*Backporting Rhea to Hawaii End*/
/*
	if(state->fps == FRAME_RATE_AUTO) {
		CAM_ERROR_PRINTK("%s : Rollback to FRAME_RATE_AUTO again as capture is done !!\n",__func__);

		if (sensor.fps_auto_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : fps_auto_regs not supported !!\n", sensor.name, __func__);
		} else {
			err =  camdrv_ss_i2c_set_config_register(client, sensor.fps_auto_regs, sensor.rows_num_fps_auto_regs, "fps_auto_regs");
			if(err < 0)
				CAM_ERROR_PRINTK("%s FRAME RATE AUTO SETTING ERROR ! FAILED !\n", __func__);
		}
	}
*/
	atomic_set(&gCapModeState, CAMDRV_SS_CAPTURE_MODE_NOT_SUPPORT);
	return 0;
}


static int camdrv_ss_set_scene_mode(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s %s :   value =%d\n", sensor.name, __func__, ctrl->val);
#if 1
	if (state->current_mode == PICTURE_MODE) {
		return 0;
	}
#endif
	if (ctrl->val != SCENE_MODE_NONE) {
		if (sensor.scene_none_regs == 0)
			CAM_ERROR_PRINTK("%s %s : scene_none_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_none_regs, sensor.rows_num_scene_none_regs, "scene_none_regs");
		if (err < 0) {
			CAM_ERROR_PRINTK("[%s : %d] ERROR! Could not take a picture\n", __FILE__, __LINE__);
			return -EIO;
		}
	}

	switch (ctrl->val) {
	case SCENE_MODE_NONE:
	{
		if (sensor.scene_none_regs == 0)
			CAM_ERROR_PRINTK("%s %s : scene_none_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_none_regs, sensor.rows_num_scene_none_regs, "scene_none_regs");
		break;
	}

	case SCENE_MODE_PORTRAIT:
	{
		/* Metering-Center, EV0, WB-Auto, Sharp-1, Sat0, AF-Auto will be set in HAL layer */

		if (sensor.scene_portrait_regs == 0)
			CAM_ERROR_PRINTK("%s %s : scene_portrait_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_portrait_regs, sensor.rows_num_scene_portrait_regs, "scene_portrait_regs");
		break;
	}

	case SCENE_MODE_NIGHTSHOT:
	{
		if (sensor.scene_nightshot_regs == 0) {
			CAM_ERROR_PRINTK("%s %s : scene_nightshot_regs not supported !!!\n", sensor.name, __func__);
		} else {
			/* aska add */
			if (sensor.get_nightmode(sd)) {
				CAM_ERROR_PRINTK("night mode dark\n");
				err = camdrv_ss_i2c_set_config_register(client, sensor.scene_nightshot_dark_regs, sensor.rows_num_scene_nightshot_dark_regs, "scene_nightshot_dark_regs");
			} else {
				CAM_ERROR_PRINTK("night mode normal\n");
				err = camdrv_ss_i2c_set_config_register(client, sensor.scene_nightshot_regs, sensor.rows_num_scene_nightshot_regs, "scene_nightshot_regs");
			}
		}
		break;
	}

	case SCENE_MODE_BACK_LIGHT:
	{
		/* Metering-Spot, EV0, WB-Auto, Sharp0, Sat0, AF-Auto will be set in HAL layer */
		if (sensor.scene_backlight_regs == 0)
			CAM_ERROR_PRINTK("%s %s : scene_backlight_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_backlight_regs, sensor.rows_num_scene_backlight_regs, "scene_backlight_regs");
		break;
	}

	case SCENE_MODE_LANDSCAPE:
	{
		/* Metering-Matrix, EV0, WB-Auto, Sharp+1, Sat+1, AF-Auto will be set in HAL layer */

		if (sensor.scene_landscape_regs == 0)
			CAM_ERROR_PRINTK("%s %s : scene_landscape_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_landscape_regs, sensor.rows_num_scene_landscape_regs, "scene_landscape_regs");

		break;
	}

	case SCENE_MODE_SPORTS:
	{
		if (sensor.scene_sports_regs == 0)
			CAM_ERROR_PRINTK("%s %s : scene_sports_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_sports_regs, sensor.rows_num_scene_sports_regs, "scene_sports_regs");

		break;
	}

	case SCENE_MODE_PARTY_INDOOR:
	{
		if (sensor.scene_party_indoor_regs == 0)
			CAM_ERROR_PRINTK("%s %s : scene_party_indoor_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_party_indoor_regs, sensor.rows_num_scene_party_indoor_regs, "scene_party_indoor_regs");

		break;
	}

	case SCENE_MODE_BEACH_SNOW:
	{
		/* Metering-Center, EV+1, WB-Auto, Sharp0, Sat+1, AF-Auto will be set in HAL layer */
		if (sensor.scene_beach_snow_regs == 0)
			CAM_ERROR_PRINTK("%s %s : saturation_plus_1_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_beach_snow_regs, sensor.rows_num_scene_beach_snow_regs, "scene_beach_snow_regs");

		break;
	}

	case SCENE_MODE_SUNSET:
	{
		/* Metering-Center, EV0, WB-daylight, Sharp0, Sat0, AF-Auto will be set in HAL layer */
		if (sensor.scene_sunset_regs == 0)
			CAM_ERROR_PRINTK("%s %s : scene_sunset_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_sunset_regs, sensor.rows_num_scene_sunset_regs, "scene_sunset_regs");
		break;
	}

	case SCENE_MODE_DUSK_DAWN:
	{
		/* Metering-Center, EV0, WB-fluorescent, Sharp0, Sat0, AF-Auto will be set in HAL layer */
		if (sensor.scene_duskdawn_regs == 0)
			CAM_ERROR_PRINTK("%s %s : scene_duskdawn_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_duskdawn_regs, sensor.rows_num_scene_duskdawn_regs, "scene_duskdawn_regs");

		break;
	}

	case SCENE_MODE_FALL_COLOR:
	{
		/* Metering-Center, EV0, WB-Auto, Sharp0, Sat+2, AF-Auto will be set in HAL layer */
		if (sensor.scene_fall_color_regs == 0)
			CAM_ERROR_PRINTK("%s %s : scene_fall_color_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_fall_color_regs, sensor.rows_num_scene_fall_color_regs, "scene_fall_color_regs");

		break;
	}

	case SCENE_MODE_FIREWORKS:
	{
		if (sensor.scene_fireworks_regs == 0)
			CAM_ERROR_PRINTK("%s %s : scene_fireworks_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_fireworks_regs, sensor.rows_num_scene_fireworks_regs, "scene_fireworks_regs");

		break;
	}

	case SCENE_MODE_TEXT:
	{
		/* Metering-Center, EV0, WB-Auto, Sharp+2, Sat0, AF-Macro will be set in HAL layer */
		if (sensor.scene_text_regs == 0)
			CAM_ERROR_PRINTK("%s %s : scene_text_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_text_regs, sensor.rows_num_scene_text_regs, "scene_text_regs");

		break;
	}

	case SCENE_MODE_CANDLE_LIGHT:
	{
		/* Metering-Center, EV0, WB-Daylight, Sharp0, Sat0, AF-Auto will be set in HAL layer */
		if (sensor.scene_candle_light_regs == 0)
			CAM_ERROR_PRINTK("%s %s : scene_candle_light_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_candle_light_regs, sensor.rows_num_scene_candle_light_regs, "scene_candle_light_regs");

		break;
	}

	default:
	{
		CAM_ERROR_PRINTK("%s %s : default not supported !!!\n", sensor.name, __func__);
		err = -EINVAL;
		break;
	}
	}

	state->currentScene = ctrl->val;

	return err;
}


static int camdrv_ss_set_effect(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;
	struct camdrv_ss_state *state = to_state(sd);
	CAM_INFO_PRINTK("%s %s :  value =%d\n", sensor.name, __func__, ctrl->val);

	switch (ctrl->val) {
	case IMAGE_EFFECT_NONE:
	{
		if (sensor.effect_normal_regs == 0)
			CAM_ERROR_PRINTK("%s %s : effect_normal_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_normal_regs, sensor.rows_num_effect_normal_regs, "effect_normal_regs");

		break;
	}

	case IMAGE_EFFECT_MONO:
	{
		if (sensor.effect_mono_regs == 0)
			CAM_ERROR_PRINTK("%s %s : effect_mono_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_mono_regs, sensor.rows_num_effect_mono_regs, "effect_mono_regs");

	break;
	}

	case IMAGE_EFFECT_SEPIA:
	case IMAGE_EFFECT_ANTIQUE:
	{
		if (sensor.effect_sepia_regs == 0)
			CAM_ERROR_PRINTK("%s %s : effect_sepia_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_sepia_regs, sensor.rows_num_effect_sepia_regs, "effect_sepia_regs");

		break;
	}

	case IMAGE_EFFECT_NEGATIVE:
	{
		if (sensor.effect_negative_regs == 0)
			CAM_ERROR_PRINTK("%s %s : effect_negative_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_negative_regs, sensor.rows_num_effect_negative_regs, "effect_negative_regs");

		break;
	}

	case IMAGE_EFFECT_AQUA:
	{
		if (sensor.effect_aqua_regs == 0)
			CAM_ERROR_PRINTK("%s %s : effect_aqua_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_aqua_regs, sensor.rows_num_effect_aqua_regs, "effect_aqua_regs");

		break;
	}

	case IMAGE_EFFECT_BNW:
	{
		if (sensor.effect_black_white_regs == 0)
			CAM_ERROR_PRINTK("%s %s : effect_black_white_regsnot supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_black_white_regs, sensor.rows_num_effect_black_white_regs, "effect_black_white_regs");

		break;
	}

	case IMAGE_EFFECT_SOLARIZATION:
	{
		if (sensor.effect_solarization_regs == 0)
			CAM_ERROR_PRINTK("%s %s : effect_solarization_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_solarization_regs, sensor.rows_num_effect_solarization_regs, "effect_solarization_regs");

		break;
	}

	case IMAGE_EFFECT_SHARPEN:
	{
		if (sensor.effect_sharpen_regs == 0)
			CAM_ERROR_PRINTK("%s %s : effect_sharpen_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_sharpen_regs, sensor.rows_num_effect_sharpen_regs, "effect_sharpen_regs");

		break;
	}
	case IMAGE_EFFECT_EMBOSS:
	{
		if (sensor.effect_emboss_regs == 0)
			CAM_ERROR_PRINTK("%s %s : effect_emboss_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_emboss_regs, sensor.rows_num_effect_emboss_regs, "effect_emboss_regs");

		break;
	}
	case IMAGE_EFFECT_OUTLINE:
	{
		if (sensor.effect_outline_regs == 0)
			CAM_ERROR_PRINTK("%s %s : effect_outline_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_outline_regs, sensor.rows_num_effect_outline_regs, "effect_outline_regs");

		break;
	}

	default:
	{
		CAM_ERROR_PRINTK("%s %s : default case not supported !!!\n", sensor.name, __func__);
		break;
	}
	}

	state->currentEffect=ctrl->val;
	return err;
}


static int camdrv_ss_set_white_balance(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s %s :  value =%d\n", sensor.name, __func__, ctrl->val);

	switch (ctrl->val) {
	case WHITE_BALANCE_AUTO:
	{
		if (sensor.wb_auto_regs == 0)
			CAM_ERROR_PRINTK("%s %s : wb_auto_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_auto_regs, sensor.rows_num_wb_auto_regs, "wb_auto_regs");

		break;
	}

	case WHITE_BALANCE_SUNNY:
	{
		if (sensor.wb_sunny_regs == 0)
			CAM_ERROR_PRINTK("%s %s : wb_sunny_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_sunny_regs, sensor.rows_num_wb_sunny_regs, "wb_sunny_regs");

		break;
	}

	case WHITE_BALANCE_CLOUDY:
	{
		if (sensor.wb_cloudy_regs == 0)
			CAM_ERROR_PRINTK("%s %s : wb_cloudy_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_cloudy_regs, sensor.rows_num_wb_cloudy_regs, "wb_cloudy_regs");

		break;
	}

	case WHITE_BALANCE_TUNGSTEN:  /* WHITE_BALANCE_INCANDESCENT: */
	{
		if (sensor.wb_tungsten_regs == 0)
			CAM_ERROR_PRINTK("%s %s : wb_tungsten_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_tungsten_regs, sensor.rows_num_wb_tungsten_regs, "wb_tungsten_regs");

		break;
	}

	case WHITE_BALANCE_FLUORESCENT:
	{
		if (sensor.wb_fluorescent_regs == 0)
			CAM_ERROR_PRINTK("%s %s : wb_fluorescent_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_fluorescent_regs, sensor.rows_num_wb_fluorescent_regs, "wb_fluorescent_regs");

		break;
	}

	case WHITE_BALANCE_CWF:
	{
		if (sensor.wb_cwf_regs == 0)
			CAM_ERROR_PRINTK("%s %s : wb_cwf_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_cwf_regs, sensor.rows_num_wb_cwf_regs, "wb_cwf_regs");

		break;
	}
	case WHITE_BALANCE_DAYLIGHT:
	{
		if (sensor.wb_daylight_regs == 0)
			CAM_ERROR_PRINTK("%s %s : wb_daylight_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_daylight_regs, sensor.rows_num_wb_daylight_regs, "wb_daylight_regs");

		break;
	}
	case WHITE_BALANCE_INCANDESCENT:
	{
		if (sensor.wb_incandescent_regs == 0)
			CAM_ERROR_PRINTK("%s %s : wb_incandescent_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_incandescent_regs, sensor.rows_num_wb_incandescent_regs, "wb_incandescent_regs");

		break;
	}
	case WHITE_BALANCE_SHADE:
	{
		if (sensor.wb_shade_regs == 0)
			CAM_ERROR_PRINTK("%s %s : wb_shade_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_shade_regs, sensor.rows_num_wb_shade_regs, "wb_shade_regs");

		break;
	}
	case WHITE_BALANCE_HORIZON:
	{
		if (sensor.wb_horizon_regs == 0)
			CAM_ERROR_PRINTK("%s %s : wb_horizon_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_horizon_regs, sensor.rows_num_wb_horizon_regs, "wb_horizon_regs");

		break;
	}
	default:
	{
		CAM_ERROR_PRINTK("%s %s : default not supported !!!\n", sensor.name, __func__);
		break;
	}
	}

	state->currentWB = ctrl->val;

	return err;
}


static int camdrv_ss_set_metering(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s %s :  value =%d\n", sensor.name, __func__, ctrl->val);

	switch (ctrl->val) {
	case METERING_MATRIX:
	{
		if (sensor.metering_matrix_regs == 0)
			CAM_ERROR_PRINTK("%s %s : metering_matrix_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.metering_matrix_regs, sensor.rows_num_metering_matrix_regs, "metering_matrix_regs");

		break;
	}

	case METERING_CENTER:
	{
		if (sensor.metering_center_regs == 0)
			CAM_ERROR_PRINTK("%s %s : metering_center_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.metering_center_regs, sensor.rows_num_metering_center_regs, "metering_center_regs");

		break;
	}

	case METERING_SPOT:
	{

		if (sensor.metering_spot_regs == 0)
			CAM_ERROR_PRINTK("%s %s : metering_spot_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.metering_spot_regs, sensor.rows_num_metering_spot_regs, "metering_spot_regs");

		break;

	}

	default:
	{
		CAM_ERROR_PRINTK("%s %s : default not supported !!!\n", sensor.name, __func__);
		break;
	}
	}

	state->currentMetering = ctrl->val;

	return err;
}

#if 0
static int camdrv_ss_set_iso(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;


	CAM_INFO_PRINTK("%s %s :  value =%d\n", sensor.name, __func__, ctrl->val);

	switch (ctrl->val) {
	case ISO_AUTO:
	{
		if (sensor.iso_auto_regs == 0)
			CAM_ERROR_PRINTK("%s %s : iso_auto_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.iso_auto_regs, sensor.rows_num_iso_auto_regs, "iso_auto_regs");

		break;
	}
	case ISO_50:
	{
		if (sensor.iso_50_regs == 0)
			CAM_ERROR_PRINTK("%s %s : iso_50_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.iso_50_regs, sensor.rows_num_iso_50_regs, "iso_50_regs");

		break;
	}

	case ISO_100:
	{
		if (sensor.iso_100_regs == 0)
			CAM_ERROR_PRINTK("%s %s : iso_100_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.iso_100_regs, sensor.rows_num_iso_100_regs, "iso_100_regs");

		break;
	}
	case ISO_200:
	{
		if (sensor.iso_200_regs == 0)
			CAM_ERROR_PRINTK("%s %s : iso_200_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.iso_200_regs, sensor.rows_num_iso_200_regs, "iso_200_regs");

		break;
	}
	case ISO_400:
	{
		if (sensor.iso_400_regs == 0)
			CAM_ERROR_PRINTK("%s %s : iso_400_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.iso_400_regs, sensor.rows_num_iso_400_regs, "iso_400_regs");

		break;
	}

	case ISO_800:
	{
		if (sensor.iso_800_regs == 0)
			CAM_ERROR_PRINTK("%s %s : iso_800_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.iso_800_regs, sensor.rows_num_iso_800_regs, "iso_800_regs");

		break;
	}

	case ISO_1200:
	{
		if (sensor.iso_1200_regs == 0)
			CAM_ERROR_PRINTK("%s %s : iso_1200_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.iso_1200_regs, sensor.rows_num_iso_1200_regs, "iso_1200_regs");

		break;
	}

	case ISO_1600:
	{
		if (sensor.iso_1600_regs == 0)
			CAM_ERROR_PRINTK("%s %s : iso_1600_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.iso_1600_regs, sensor.rows_num_iso_1600_regs, "iso_1600_regs");

		break;
	}

	case ISO_2400:
	{
		if (sensor.iso_2400_regs == 0)
			CAM_ERROR_PRINTK("%s %s : iso_2400_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.iso_2400_regs, sensor.rows_num_iso_2400_regs, "iso_2400_regs");

		break;
	}

	case ISO_3200:
	{
		if (sensor.iso_3200_regs == 0)
			CAM_ERROR_PRINTK("%s %s : iso_3200_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.iso_3200_regs, sensor.rows_num_iso_3200_regs, "iso_3200_regs");

		break;
	}

	case ISO_SPORTS:
	{
		if (sensor.iso_sports_regs == 0)
			CAM_ERROR_PRINTK("%s %s : iso_sports_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.iso_sports_regs, sensor.rows_num_iso_sports_regs, "iso_sports_regs");

		break;
	}

	case ISO_NIGHT:
	{
		if (sensor.iso_night_regs == 0)
			CAM_ERROR_PRINTK("%s %s : iso_night_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.iso_night_regs, sensor.rows_num_iso_night_regs, "iso_night_regs");

		break;
	}

	case ISO_MOVIE:
	{
		if (sensor.iso_movie_regs == 0)
			CAM_ERROR_PRINTK("%s %s : iso_movie_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.iso_movie_regs, sensor.rows_num_iso_movie_regs, "iso_movie_regs");

		break;
	}

	default:
	{
		CAM_ERROR_PRINTK("%s %s : default case supported !!!\n", sensor.name, __func__);
		break;
	}
	} /* end of switch */

	return err;
}
#endif /* #if 0 */

static int camdrv_ss_set_ev(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	int err = 0;
	/* struct camdrv_ss_state *state = to_state(sd); */
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	CAM_INFO_PRINTK("%s %s :  value =%d\n", sensor.name, __func__, ctrl->val);

	switch (ctrl->val) {
	case EV_MINUS_4:
	{
		if (sensor.ev_minus_4_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ev_minus_4_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_minus_4_regs, sensor.rows_num_ev_minus_4_regs, "ev_minus_4_regs");

		break;
	}

	case EV_MINUS_3:
	{
		if (sensor.ev_minus_3_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ev_minus_3_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_minus_3_regs, sensor.rows_num_ev_minus_3_regs, "ev_minus_3_regs");

		break;
	}

	case EV_MINUS_2:
	{
		if (sensor.ev_minus_2_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ev_minus_2_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_minus_2_regs, sensor.rows_num_ev_minus_2_regs, "ev_minus_2_regs");

		break;
	}
	case EV_MINUS_1_5:
	{
		if (sensor.ev_minus_1_5_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ev_minus_1_5_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_minus_1_5_regs, sensor.rows_num_ev_minus_1_5_regs, "ev_minus_1_5_regs");

		break;
	}

	case EV_MINUS_1:
	{
		if (sensor.ev_minus_1_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ev_minus_1_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_minus_1_regs, sensor.rows_num_ev_minus_1_regs, "ev_minus_1_regs");

		break;
	}
	case EV_MINUS_0_5:
	{
		if (sensor.ev_minus_0_5_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ev_minus_0_5_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_minus_0_5_regs, sensor.rows_num_ev_minus_0_5_regs, "ev_minus_0_5_regs");

		break;
	}
	case EV_DEFAULT:
	{
		if (sensor.ev_default_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ev_default_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_default_regs, sensor.rows_num_ev_default_regs, "ev_default_regs");

		break;
	}
	case EV_PLUS_0_5:
	{
		if (sensor.ev_plus_0_5_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ev_plus_0_5_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_plus_0_5_regs, sensor.rows_num_ev_plus_0_5_regs, "ev_plus_0_5_regs");

		break;
	}
	case EV_PLUS_1:
	{
		if (sensor.ev_plus_1_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ev_plus_1_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_plus_1_regs, sensor.rows_num_ev_plus_1_regs, "ev_plus_1_regs");

		break;
	}
	case EV_PLUS_1_5:
	{
		if (sensor.ev_plus_1_5_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ev_plus_1_5_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client,
					sensor.ev_plus_1_5_regs,
					sensor.rows_num_ev_plus_1_5_regs,
					"ev_plus_1_5_regs");

		break;
	}

	case EV_PLUS_2:
	{
		if (sensor.ev_plus_2_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ev_plus_2_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_plus_2_regs, sensor.rows_num_ev_plus_2_regs, "ev_plus_2_regs");

		break;
	}
	case EV_PLUS_3:
	{
		if (sensor.ev_plus_3_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ev_plus_3_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_plus_3_regs, sensor.rows_num_ev_plus_3_regs, "ev_plus_3_regs");

		break;
	}

	case EV_PLUS_4:
	{
		if (sensor.ev_plus_4_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ev_plus_4_regs not supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_plus_4_regs, sensor.rows_num_ev_plus_4_regs, "ev_plus_4_regs");

		break;
	}

	default:
	{
		CAM_ERROR_PRINTK("%s %s : default case supported !!!\n", sensor.name, __func__);
		break;
	}
	}

	return err;
}


static int camdrv_ss_set_saturation(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s %s :  value =%d\n", sensor.name, __func__, ctrl->val);

	switch (ctrl->val) {
	case SATURATION_MINUS_2:
	{
		if (sensor.saturation_minus_2_regs == 0)
			CAM_ERROR_PRINTK("%s %s : saturation_minus_2_regs supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.saturation_minus_2_regs, sensor.rows_num_saturation_minus_2_regs, "saturation_minus_2_regs");

		break;
	}

	case SATURATION_MINUS_1:
	{
		if (sensor.saturation_minus_1_regs == 0)
			CAM_ERROR_PRINTK("%s %s : saturation_minus_1_regs supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.saturation_minus_1_regs, sensor.rows_num_saturation_minus_1_regs, "saturation_minus_1_regs");

		break;
	}

	case SATURATION_DEFAULT:
	{
		if (sensor.saturation_default_regs == 0)
			CAM_ERROR_PRINTK("%s %s : saturation_default_regs supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.saturation_default_regs, sensor.rows_num_saturation_default_regs, "saturation_default_regs");

		break;
	}

	case SATURATION_PLUS_1:
	{
		if (sensor.saturation_plus_1_regs == 0)
			CAM_ERROR_PRINTK("%s %s :  saturation_plus_1_regs supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.saturation_plus_1_regs, sensor.rows_num_saturation_plus_1_regs, "saturation_plus_1_regs");

		break;
	}

	case SATURATION_PLUS_2:
	{
		if (sensor.saturation_plus_2_regs == 0)
			CAM_ERROR_PRINTK("%s %s : saturation_plus_2_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.saturation_plus_2_regs, sensor.rows_num_saturation_plus_2_regs, "saturation_plus_2_regs");

		break;
	}

	default:
	{
		CAM_ERROR_PRINTK("%s %s : default case  supported !!!\n", sensor.name, __func__);
		break;
	}
	}

	return err;
}


static int camdrv_ss_set_contrast(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s %s :  value =%d\n", sensor.name, __func__, ctrl->val);

	switch (ctrl->val) {
	case CONTRAST_MINUS_2:
	{
		if (sensor.contrast_minus_2_regs == 0)
			CAM_ERROR_PRINTK("%s %s : contrast_minus_2_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.contrast_minus_2_regs, sensor.rows_num_contrast_minus_2_regs, "contrast_minus_2_regs");

		break;
	}

	case CONTRAST_MINUS_1:
	{
		if (sensor.contrast_minus_1_regs == 0)
			CAM_ERROR_PRINTK("%s %s : contrast_minus_1_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.contrast_minus_1_regs, sensor.rows_num_contrast_minus_1_regs, "contrast_minus_1_regs");

		break;
	}

	case CONTRAST_DEFAULT:
	{
		if (sensor.contrast_default_regs == 0)
			CAM_ERROR_PRINTK("%s %s : contrast_default_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.contrast_default_regs, sensor.rows_num_contrast_default_regs, "contrast_default_regs");

		break;
	}

	case CONTRAST_PLUS_1:
	{
		if (sensor.contrast_plus_1_regs == 0)
			CAM_ERROR_PRINTK("%s %s : contrast_plus_1_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.contrast_plus_1_regs, sensor.rows_num_contrast_plus_1_regs, "contrast_plus_1_regs");

		break;
	}

	case CONTRAST_PLUS_2:
	{
		if (sensor.contrast_plus_2_regs == 0)
			CAM_ERROR_PRINTK("%s %s : contrast_plus_2_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.contrast_plus_2_regs, sensor.rows_num_contrast_plus_2_regs, "contrast_plus_2_regs");

		break;

	}

	default:
	{
		CAM_ERROR_PRINTK("%s %s : default case  supported !!!\n", sensor.name, __func__);
		break;
	}
	}


	return err;
}

static int camdrv_ss_set_sharpness(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s %s :  value =%d\n", sensor.name, __func__, ctrl->val);

	switch (ctrl->val) {
	case SHARPNESS_MINUS_2:
	{
		if (sensor.sharpness_minus_2_regs == 0)
			CAM_ERROR_PRINTK("%s %s : sharpness_minus_2_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.sharpness_minus_2_regs, sensor.rows_num_sharpness_minus_2_regs, "sharpness_minus_2_regs");

		break;
	}

	case SHARPNESS_MINUS_1:
	{
		if (sensor.sharpness_minus_1_regs == 0)
			CAM_ERROR_PRINTK("%s %s : sharpness_minus_1_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.sharpness_minus_1_regs, sensor.rows_num_sharpness_minus_1_regs, "sharpness_minus_1_regs");

		break;
	}

	case SHARPNESS_DEFAULT:
	{
		if (sensor.sharpness_default_regs == 0)
			CAM_ERROR_PRINTK("%s %s : sharpness_default_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.sharpness_default_regs, sensor.rows_num_sharpness_default_regs, "sharpness_default_regs");

		break;
	}

	case SHARPNESS_PLUS_1:
	{
		if (sensor.sharpness_plus_1_regs == 0)
			CAM_ERROR_PRINTK("%s %s : sharpness_plus_1_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.sharpness_plus_1_regs, sensor.rows_num_sharpness_plus_1_regs, "sharpness_plus_1_regs");

		break;
	}

	case SHARPNESS_PLUS_2:
	{
		if (sensor.sharpness_plus_2_regs == 0)
			CAM_ERROR_PRINTK("%s %s : sharpness_plus_2_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.sharpness_plus_2_regs, sensor.rows_num_sharpness_plus_2_regs, "sharpness_plus_2_regs");

		break;
	}

	default:
	{
		CAM_ERROR_PRINTK("%s %s : default case  supported !!!\n", sensor.name, __func__);
		break;
	}
	} /* end of switch */

	return err;
}


static int camdrv_ss_set_focus_mode(struct v4l2_subdev *sd, int value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s %s :  value =%d\n", sensor.name, __func__, value);

	switch (value) {
	case FOCUS_MODE_OFF:
	{
		if (sensor.focus_mode_off_regs == 0)
			CAM_ERROR_PRINTK("%s %s : focus_mode_off_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.focus_mode_off_regs, sensor.rows_num_focus_mode_off_regs, "focus_mode_off_regs");

		break;
	}
	case FOCUS_MODE_AF:
	{
		if (sensor.focus_mode_af_regs == 0)
			CAM_ERROR_PRINTK("%s %s : focus_mode_af_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.focus_mode_af_regs, sensor.rows_num_focus_mode_af_regs, "focus_mode_af_regs");

		break;
	}
	case FOCUS_MODE_MACRO:
	{
		if (sensor.focus_mode_macro_regs == 0)
			CAM_ERROR_PRINTK("%s %s : focus_mode_macro_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.focus_mode_macro_regs, sensor.rows_num_focus_mode_macro_regs, "focus_mode_macro_regs");

		break;
	}
	case FOCUS_MODE_FACEDETECT:
	{
		if (sensor.focus_mode_facedetect_regs == 0)
			CAM_ERROR_PRINTK("%s %s : focus_mode_facedetect_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.focus_mode_facedetect_regs, sensor.rows_num_focus_mode_facedetect_regs, "focus_mode_facedetect_regs");

		break;
	}
	case FOCUS_MODE_INFINITY:
	{
		if (sensor.focus_mode_infinity_regs == 0)
			CAM_ERROR_PRINTK("%s %s : focus_mode_infinity_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.focus_mode_infinity_regs, sensor.rows_num_focus_mode_infinity_regs, "focus_mode_infinity_regs");

		break;
	}

	case FOCUS_MODE_AUTO:
	{
		if (sensor.focus_mode_auto_regs == 0)
			CAM_ERROR_PRINTK("%s %s : focus_mode_auto_regs  supported !!!\n", sensor.name, __func__);
		else
			err |= camdrv_ss_i2c_set_config_register(client, sensor.focus_mode_auto_regs, sensor.rows_num_focus_mode_auto_regs, "focus_mode_auto_regs");

		break;
	}
	case FOCUS_MODE_CONTINUOUS_VIDEO:
	{
		if (sensor.focus_mode_continuous_video_regs == 0)
			CAM_ERROR_PRINTK("%s %s : focus_mode_continuous_video_regs  supported !!!\n", sensor.name, __func__);
		else
			err |= camdrv_ss_i2c_set_config_register(client, sensor.focus_mode_continuous_video_regs, sensor.rows_num_focus_mode_continuous_video_regs, "focus_mode_continuous_video_regs");

		break;
	}
	case FOCUS_MODE_CONTINUOUS_PICTURE:
	{
		if (sensor.focus_mode_continuous_picture_regs == 0)
			CAM_ERROR_PRINTK("%s %s : focus_mode_continuous_picture_regs  supported !!!\n", sensor.name, __func__);
		else
			err |= camdrv_ss_i2c_set_config_register(client, sensor.focus_mode_continuous_picture_regs, sensor.rows_num_focus_mode_continuous_picture_regs, "focus_mode_continuous_picture_regs");

		break;
	}
	case FOCUS_MODE_CONTINUOUS_PICTURE_MACRO:
	{
		if (sensor.focus_mode_continuous_picture_macro_regs == 0)
			CAM_ERROR_PRINTK("%s %s : focus_mode_continuous_picture_macro_regs  supported !!!\n", sensor.name, __func__);
		else
			err |= camdrv_ss_i2c_set_config_register(client, sensor.focus_mode_continuous_picture_macro_regs, sensor.rows_num_focus_mode_continuous_picture_macro_regs, "focus_mode_continuous_picture_macro_regs");

		break;
	}

	default:
	{
		CAM_ERROR_PRINTK("%s %s : default case not  supported !!!\n", sensor.name, __func__);
		break;
	}
	}

	state->af_mode = value;

	return err;
}

static int camdrv_ss_set_af_preflash(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct camdrv_ss_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s %s : entered\n", sensor.name, __func__);

	if (ctrl->val == PREFLASH_ON) {
		state->camera_af_flash_fire = 0;

		if (state->current_flash_mode == FLASH_MODE_ON) {
			state->camera_af_flash_fire = 1;
		} else if (state->current_flash_mode == FLASH_MODE_AUTO) {
			bool bflash_needed = false;

			if (sensor.check_flash_needed == 0)
				CAM_ERROR_PRINTK("%s %s : check_flash_needed NULL!! not supported !!!\n", sensor.name, __func__);
			else
				bflash_needed = sensor.check_flash_needed(sd);

			if (bflash_needed) {
				state->camera_af_flash_fire = 1;
			}
		}

		/* Synchronize main flash with pre-flash. */
		state->camera_flash_fire = state->camera_af_flash_fire;

		/* Support 3rd Party App without AF Pre-Flash. */
		state->camera_af_flash_checked = 1;

		if (state->camera_af_flash_fire) {

			if (sensor.snapshot_af_preflash_on_regs == 0)
				CAM_ERROR_PRINTK("%s %s : snapshot_af_preflash_on_regs NOT  supported !!!\n", sensor.name, __func__);
			else
				err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_af_preflash_on_regs, sensor.rows_num_snapshot_af_preflash_on_regs, "snapshot_af_preflash_on_regs");
			if (err < 0) {
				CAM_ERROR_PRINTK("[%s: %d] ERROR! Setting af_preflash_on_regs\n", __FILE__, __LINE__);
			}

			if (sensor.flash_control == NULL)
				CAM_ERROR_PRINTK("%s %s : flash_control  NULL!!not  supported !!!\n", sensor.name, __func__);
			else
				sensor.flash_control(sd, FLASH_CONTROL_MIDDLE_LEVEL);
		} else {
			return -1; /* Must return a non-zero value, when flash is not fired. */
		}
	} else { /* if (ctrl->val == PREFLASH_ON) */
		if (state->camera_af_flash_fire) {
			if (sensor.snapshot_af_preflash_off_regs == NULL) {
				CAM_ERROR_PRINTK("%s %s : snapshot_af_preflash_off_regs NOT supported !!!\n", sensor.name, __func__);
			} else {
				err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_af_preflash_off_regs, sensor.rows_num_snapshot_af_preflash_off_regs, "snapshot_af_preflash_off_regs");
				if (err < 0) {
					CAM_ERROR_PRINTK("[%s: %d] ERROR! Setting af_preflash_off_regs\n", __FILE__, __LINE__);
				}
			}
			if (sensor.flash_control == NULL)
				CAM_ERROR_PRINTK("%s %s : flash_control  NULL!!not  supported !!!\n", sensor.name, __func__);
			else
				sensor.flash_control(sd, FLASH_CONTROL_OFF);

			state->camera_af_flash_fire = 0;
		}
	}

	return 0;
}

static int camdrv_ss_set_autocontrast(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	if (ctrl->val) {
		/* on */
		CAM_INFO_PRINTK("%s %s :ON !!\n", sensor.name, __func__);

		if (sensor.auto_contrast_on_regs == NULL)
			CAM_ERROR_PRINTK("%s %s : auto_contrast_on_regs is NULL, please check if it is needed !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.auto_contrast_on_regs, sensor.rows_num_auto_contrast_on_regs, "auto_contrast_on_regs");

		if (err < 0) {
			CAM_ERROR_PRINTK("%s %s : i2c failed !!\n", sensor.name, __func__);
			return -EIO;
		}
	} else {
		/*  off */
		CAM_INFO_PRINTK("%s %s :OFF !!\n", sensor.name, __func__);

		if (sensor.auto_contrast_off_regs == NULL)
			CAM_ERROR_PRINTK("%s %s : auto_contrast_off_regs is NULL, please check if it is needed !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.auto_contrast_off_regs, sensor.rows_num_auto_contrast_off_regs, "auto_contrast_off_regs");

		if (err < 0) {
			CAM_ERROR_PRINTK("%s %s : i2c failed !!\n", sensor.name, __func__);
			return -EIO;
		}

		/* state->check_dataline = CHK_DATALINE_OFF; */
	}

	CAM_INFO_PRINTK("%s %s :done .\n", sensor.name, __func__);

	return err;
}


static int camdrv_ss_AE_AWB_lock(struct v4l2_subdev *sd, struct v4l2_ctrl *ctrl)
{
	struct camdrv_ss_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_PRINTK("%s %s :  value=%d\n", sensor.name, __func__, ctrl->val);

	/* Lock, Unlock only AE for LSI 5CC sensor. Don't change AWB. */
	switch (ctrl->val) {
	case AE_UNLOCK_AWB_UNLOCK:
	{
		if (sensor.ae_unlock_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ae_unlock_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ae_unlock_regs, sensor.rows_num_ae_unlock_regs, "ae_unlock_regs");

		if (sensor.awb_unlock_regs == 0)
			CAM_ERROR_PRINTK("%s %s : awb_unlock_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.awb_unlock_regs, sensor.rows_num_awb_unlock_regs, "awb_unlock_regs");
		break;
	}

	case AE_LOCK_AWB_UNLOCK:
	{
		if (sensor.ae_lock_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ae_lock_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ae_lock_regs, sensor.rows_num_ae_lock_regs, "ae_lock_regs");

		if (sensor.awb_unlock_regs == 0)
			CAM_ERROR_PRINTK("%s %s : awb_unlock_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.awb_unlock_regs, sensor.rows_num_awb_unlock_regs, "awb_unlock_regs");
		break;
	}

	case AE_UNLOCK_AWB_LOCK:
	{
		if (sensor.ae_unlock_regs == 0)
				CAM_ERROR_PRINTK("%s %s : ae_unlock_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ae_unlock_regs, sensor.rows_num_ae_unlock_regs, "ae_unlock_regs");

		if (sensor.awb_lock_regs == 0)
			CAM_ERROR_PRINTK("%s %s : awb_lock_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.awb_lock_regs, sensor.rows_num_awb_lock_regs, "awb_lock_regs");
		break;
	}

	case AE_LOCK_AWB_LOCK:
	{
		if (sensor.ae_lock_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ae_lock_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ae_lock_regs, sensor.rows_num_ae_lock_regs, "ae_lock_regs");

		if (!state->camera_af_flash_fire) {
			if (sensor.awb_lock_regs == 0)
				CAM_ERROR_PRINTK("%s %s : awb_lock_regs  supported !!!\n", sensor.name, __func__);
			else
				err = camdrv_ss_i2c_set_config_register(client, sensor.awb_lock_regs, sensor.rows_num_awb_lock_regs, "awb_lock_regs");
		}
		break;
	}
	case  AE_UNLOCK:
	{
		if (sensor.ae_unlock_regs == 0)
			CAM_ERROR_PRINTK("%s %s : ae_unlock_regs  supported !!!\n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ae_unlock_regs, sensor.rows_num_ae_unlock_regs, "ae_unlock_regs");

	}
	 break;
	default:
	{
		CAM_ERROR_PRINTK("[%s : %d] WARNING! Unsupported AE, AWB lock setting(%d)\n", __FILE__, __LINE__, ctrl->val);
		break;
	}
	}

	if (err < 0) {
		CAM_ERROR_PRINTK("[%s : %d] ERROR! AE, AWB lock failed\n", __FILE__, __LINE__);
		return -EIO;
	}

	return 0;
}


static void camdrv_ss_init_parameters(struct v4l2_subdev *sd)
{
	struct camdrv_ss_state *state = to_state(sd);

	/* Default value */
	/* state->preview_framesize_index = PREVIEW_SIZE_XGA; */
	state->fps = 30;
	/* Set initial values for the sensor stream parameters */
	state->strm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	state->strm.parm.capture.timeperframe.numerator = 1;
	state->strm.parm.capture.timeperframe.denominator = state->fps;
	state->strm.parm.capture.capturemode = 0;

	state->jpeg_param.enable = 0;
	state->jpeg_param.quality = 100;
	state->jpeg_param.main_offset = 0;
	state->jpeg_param.main_size = 0;
	state->jpeg_param.thumb_offset = 0;
	state->jpeg_param.thumb_size = 0;
	state->jpeg_param.postview_offset = 0;

	state->postview_info.width = 320;
	state->postview_info.height = 240;

	state->current_flash_mode = FLASH_MODE_OFF;
	state->camera_flash_fire = 0;
	state->camera_af_flash_fire = 0;
	state->camera_af_flash_checked = 0;

	state->af_mode = FOCUS_MODE_AUTO;
	state->currentScene = SCENE_MODE_NONE;
	state->currentWB = WHITE_BALANCE_AUTO;
	state->check_dataline = CHK_DATALINE_OFF;
	state->bTouchFocus = false;
	state->touch_area.leftTopX = 0;
	state->touch_area.leftTopY = 0;
	state->touch_area.rightBottomX = 0;
	state->touch_area.rightBottomY = 0;
	state->touch_area.weight = 0;

}


/*
 * Clock configuration
 * Configure expected MCLK from host and return EINVAL if not supported clock
 * frequency is expected
 *	freq : in Hz
 *	flag : not supported for now
 */

static int camdrv_ss_s_crystal_freq(struct v4l2_subdev *sd, u32 freq, u32 flags)
{
	int err = -EINVAL;

	return err;
}


static int camdrv_ss_find_preview_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < sensor.supported_number_of_preview_sizes; i++) {
		if ((sensor.supported_preview_framesize_list[i].width >= width) &&
				(sensor.supported_preview_framesize_list[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= sensor.supported_number_of_preview_sizes)
		i = sensor.supported_number_of_preview_sizes - 1;

	return i;
}


static int camdrv_ss_find_capture_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < sensor.supported_number_of_capture_sizes; i++) {
		if ((sensor.supported_capture_framesize_list[i].width >= width) &&
		    (sensor.supported_capture_framesize_list[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= sensor.supported_number_of_capture_sizes)
		i = sensor.supported_number_of_capture_sizes - 1;

	return i;
}


/* Information received:
 * width, height
 * pixel_format -> to be handled in the upper layer
 *
 * */
static int camdrv_ss_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *fmt)
{
	struct camdrv_ss_state *state = to_state(sd);
	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */
	int index = 0;
	state->pix.width = fmt->width;
	state->pix.height = fmt->height;
	state->pix.pixelformat = fmt->code;

	if ((state->pix.pixelformat == V4L2_MBUS_FMT_JPEG_1X8) || (state->pix.pixelformat == V4L2_PIX_FMT_JPEG)) {
		state->jpeg_param.enable = 1;
		index =  camdrv_ss_find_capture_framesize(state->pix.width , state->pix.height);
		fmt->width = state->pix.width = sensor.supported_capture_framesize_list[index].width;
		fmt->height = state->pix.height = sensor.supported_capture_framesize_list[index].height;
		state->capture_framesize_index = sensor.supported_capture_framesize_list[index].index;
		CAM_INFO_PRINTK("%s %s :  capture_framesize_index=%d\n", sensor.name, __func__, state->capture_framesize_index);

	} else {
		state->jpeg_param.enable = 0;

		index =  camdrv_ss_find_preview_framesize(state->pix.width , state->pix.height);
		fmt->width = state->pix.width = sensor.supported_preview_framesize_list[index].width;
		fmt->height = state->pix.height = sensor.supported_preview_framesize_list[index].height;
		state->preview_framesize_index = sensor.supported_preview_framesize_list[index].index;

		index =  camdrv_ss_find_capture_framesize(state->pix.width , state->pix.height);
		if ((fmt->width != sensor.supported_capture_framesize_list[index].width) || (fmt->height != sensor.supported_capture_framesize_list[index].height)) {
			CAM_ERROR_PRINTK("%s: Sensor didn't define capture settings for %d x %d, So ,we will take preview settings  for capture also!!!\n", __func__, fmt->width, fmt->height);
			state->capture_framesize_index = -1;
		} else {
			state->capture_framesize_index = sensor.supported_capture_framesize_list[index].index;
		}
		CAM_INFO_PRINTK("%s %s : preview_framesize_index=%d  capture_framesize_index = %d\n", sensor.name, __func__, state->preview_framesize_index, state->capture_framesize_index);

	}
	return 0;
}


static int camdrv_ss_enum_framesizes(struct v4l2_subdev *sd, struct v4l2_frmsizeenum *fsize)
{
	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */


	if (fsize->pixel_format == sensor.default_pix_fmt || fsize->pixel_format == sensor.default_mbus_pix_fmt) {
		if (fsize->index >= sensor.supported_number_of_preview_sizes) {
			CAM_ERROR_PRINTK("%s %s : exceeded index =%d\n", sensor.name, __func__, fsize->index);
			return -EINVAL;
		}

		/* The camera interface should read this value, this is the resolution
		 * at which the sensor would provide framedata to the camera i/f
		 *
		 * In case of image capture,
		 * this returns the default camera resolution (SVGA)
		 */
		fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;

		fsize->discrete.width = sensor.supported_preview_framesize_list[fsize->index].width;
		fsize->discrete.height = sensor.supported_preview_framesize_list[fsize->index].height;
	} else if (fsize->pixel_format == V4L2_PIX_FMT_JPEG || fsize->pixel_format == V4L2_MBUS_FMT_JPEG_1X8) {
		if (fsize->index >=  sensor.supported_number_of_capture_sizes) {
			CAM_ERROR_PRINTK("%s %s : exceeded index =%d\n", sensor.name, __func__, fsize->index);
			return -EINVAL;
		}

		/* The camera interface should read this value, this is the resolution
		 * at which the sensor would provide framedata to the camera i/f
		 *
		 * In case of image capture,
		 * this returns the default camera resolution (SVGA)
		 */
		fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;

		fsize->discrete.width = sensor.supported_capture_framesize_list[fsize->index].width;
		fsize->discrete.height = sensor.supported_capture_framesize_list[fsize->index].height;
	} else {
		CAM_ERROR_PRINTK("%s %s : wrong format =%d ,failed !!\n", sensor.name, __func__, fsize->index);
		return -EINVAL;
	}

	return 0;
}


static int camdrv_ss_enum_frameintervals(struct v4l2_subdev *sd,
					struct v4l2_frmivalenum *fival)
{
	return sensor.enum_frameintervals(sd, fival);
}


static int camdrv_ss_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
				enum v4l2_mbus_pixelcode *code)
{
	int num_entries = sensor.rows_num_fmts;
	if (index >= num_entries) {
		CAM_ERROR_PRINTK("%s %s : index =%d, num_entries =%d failed\n", sensor.name, __func__, index, num_entries);
		return -EINVAL;
	}

	*code = sensor.fmts[index].pixelformat;
	return 0;
}


static int camdrv_ss_try_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */

	int num_entries;
	int i;
	int ret = 0;

	num_entries = sensor.rows_num_fmts;

	for (i = 0; i < num_entries; i++) {
		if (sensor.fmts[i].pixelformat == mf->code) {
			CAM_INFO_PRINTK("%s %s : match found for =%d !!\n", sensor.name, __func__, mf->code);
			return 0;
		}
	}

	CAM_ERROR_PRINTK("%s %s : match not found for =%d  failed  !!\n", sensor.name, __func__, mf->code);

	return ret;
}

static int camdrv_ss_actual_sensor_power_up()
{
	int err = 0;
	GPIOSetup.name = PN_BSC1CLK;
	pinmux_get_pin_config(&GPIOSetup);
	GPIOSetup.func = PF_BSC1CLK;
	GPIOSetup.reg.b.pull_up = 1;
	GPIOSetup.reg.b.pull_dn = 0;
	pinmux_set_pin_config(&GPIOSetup);

	GPIOSetup.name = PN_BSC1DAT;
	pinmux_get_pin_config(&GPIOSetup);
	GPIOSetup.func = PF_BSC1DAT;
	GPIOSetup.reg.b.pull_up = 1;
	GPIOSetup.reg.b.pull_dn = 0;
	pinmux_set_pin_config(&GPIOSetup);

	err = sensor.sensor_power(1);
	if (err < 0) {
		atomic_set(&sensor_state, CAMDRV_SS_INITIALIZE_FAILED);
		CAM_ERROR_PRINTK("%s: sensor_power failed ! CAMDRV_SS_INITIALIZE_FAILED !!\n", __func__);
		return err;
	}

	return 0;

}


void camdrv_ss_init_func(struct v4l2_subdev *sd)
{
	int err;
	err = camdrv_ss_init(sd, 1);
	if (err < 0) {
		atomic_set(&sensor_state, CAMDRV_SS_INITIALIZE_FAILED);
		CAM_ERROR_PRINTK("%s:camdrv_ss_init failed! CAMDRV_SS_INITIALIZE_FAILED !!\n", __func__);
	} else {
		atomic_set(&sensor_state, CAMDRV_SS_INITIALIZE_DONE);
		CAM_INFO_PRINTK("%s: CAMDRV_SS_INITIALIZE_DONE\n", __func__);
	}
}
static int camdrv_ss_init_thread_func(void *data)
{
	struct v4l2_subdev *sd = (struct v4l2_subdev *)data;

	if (atomic_read(&sensor_state) != CAMDRV_SS_INITIALIZING) {
		CAM_ERROR_PRINTK("%s %s : incorrect camera state =%d....\n", sensor.name, __func__, atomic_read(&sensor_state));
	} else {
		CAM_INFO_PRINTK("%s %s : Initializing camera in seperate thread....\n", sensor.name, __func__);
		camdrv_ss_init_func(sd);
	}
	wake_up_interruptible(&gCamdrvReadyQ);
	return 0;
}

/* Gets current FPS value */
static int camdrv_ss_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct camdrv_ss_state *state = to_state(sd);

	int err = 0;

	state->strm.parm.capture.timeperframe.numerator = 1;
	state->strm.parm.capture.timeperframe.denominator = state->fps;

	memcpy(param, &state->strm, sizeof(*param));

	return err;
}


/* Sets the FPS value */
static int camdrv_ss_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	return 0;
}

/* Gets the EXIF information from sensor */
static int camdrv_ss_g_exif_sensor_info(
	struct v4l2_subdev *sd,
	struct v4l2_exif_sensor_info *exif_info)
{
	memset(exif_info, 0, sizeof(struct v4l2_exif_sensor_info));
	if (sensor.get_exif_sensor_info == NULL) {
		CAM_ERROR_PRINTK(
			"%s: %s, get_exif_sensor_info is NULL!!\n",
			sensor.name, __func__);
		/* return 0 to avoid java exception in app layer */
		/* NULL EXIF data will ignore in user space */
		return 0;
	}

	sensor.get_exif_sensor_info(sd, exif_info);

	return 0;
}


static int camdrv_ss_g_ctrl(struct v4l2_ctrl *ctrl)
{
	struct camdrv_ss_state *state = container_of(ctrl->handler,
					       struct camdrv_ss_state, hdl);

	struct v4l2_subdev *sd = &state->sd;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct soc_camera_subdev_desc *ssd = client->dev.platform_data;
	struct camdrv_ss_userset userset = state->userset;
	int err = 0;

	/* CAM_INFO_PRINTK("%s %s : id = %d  Entered\n", sensor.name, __func__, ctrl->id); */

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_READ_MODE_CHANGE_REG:
	{
		if (atomic_read(&gCapModeState) ==
			CAMDRV_SS_CAPTURE_MODE_READ_PROCESSING) {
			/* if successful read sensor mode change value */
			/* and that value is preview mode, skip one frame */
			/* but other case include error case, just do capture without skip frame */
			CAM_INFO_PRINTK(
			"%s %s : CAPTURE MODE STILL PROCESSING....\n",
			sensor.name, __func__);

			ctrl->val = 1;
		} else
			ctrl->val = 0;
		break;
	}
	case V4L2_CID_EXPOSURE:
	{
		ctrl->val = userset.exposure_bias;
		break;
	}
	case V4L2_CID_SENSOR_MOUNT_ORIENTATION:
	{
		struct v4l2_subdev_sensor_interface_parms *params = 
			(struct v4l2_subdev_sensor_interface_parms*)(ssd->drv_priv);

			ctrl->val = 0;
			if (params->orientation == V4L2_SUBDEV_SENSOR_PORTRAIT)
				params->orientation = V4L2_SUBDEV_SENSOR_ORIENT_0;
			if (params->orientation == V4L2_SUBDEV_SENSOR_LANDSCAPE)
				params->orientation = V4L2_SUBDEV_SENSOR_ORIENT_90;

			if (params->orientation ==
				V4L2_SUBDEV_SENSOR_ORIENT_0)
				ctrl->val &= ~(V4L2_IN_ST_HFLIP | V4L2_IN_ST_VFLIP);
			else if (params->orientation ==
				V4L2_SUBDEV_SENSOR_ORIENT_90)
				ctrl->val |= V4L2_IN_ST_HFLIP;
			else if (params->orientation ==
				V4L2_SUBDEV_SENSOR_ORIENT_180)
				ctrl->val |= V4L2_IN_ST_VFLIP;
			else if (params->orientation ==
				V4L2_SUBDEV_SENSOR_ORIENT_270)
				ctrl->val |= (V4L2_IN_ST_HFLIP | V4L2_IN_ST_VFLIP);
	
			if (params->facing == V4L2_SUBDEV_SENSOR_BACK)
				ctrl->val |= V4L2_IN_ST_BACK;
			

			CAM_INFO_PRINTK("%s %s : V4L2_CID_SENSOR_MOUNT_ORIENTATION =0x%x  \n",
						sensor.name,__func__,ctrl->val);
			break;
		
	}

	case V4L2_CID_AUTO_WHITE_BALANCE:
	{
		ctrl->val = userset.auto_wb;
		break;
	}

	case V4L2_CID_WHITE_BALANCE_PRESET:
	{
		ctrl->val = userset.manual_wb;
		break;
	}

	case V4L2_CID_COLORFX:
	{
		ctrl->val = userset.effect;
		break;
	}

	case V4L2_CID_CONTRAST:
	{
		ctrl->val = userset.contrast;
		break;
	}

	case V4L2_CID_SATURATION:
	{
		ctrl->val = userset.saturation;
		break;
	}

	case V4L2_CID_SHARPNESS:
	{
		ctrl->val = userset.sharpness;
		break;
	}

	case V4L2_CID_CAM_JPEG_MAIN_SIZE:
	{
		ctrl->val = state->jpeg_param.main_size;
		break;
	}

	case V4L2_CID_CAM_JPEG_MAIN_OFFSET:
	{
		ctrl->val = state->jpeg_param.main_offset;
		break;
	}

	case V4L2_CID_CAM_JPEG_THUMB_SIZE:
	{
		ctrl->val = state->jpeg_param.thumb_size;
		break;
	}

	case V4L2_CID_CAM_JPEG_THUMB_OFFSET:
	{
		ctrl->val = state->jpeg_param.thumb_offset;
		break;
	}

	case V4L2_CID_CAM_JPEG_POSTVIEW_OFFSET:
	{
		ctrl->val = state->jpeg_param.postview_offset;
		break;
	}

	case V4L2_CID_CAM_JPEG_MEMSIZE:
	{
		/* ctrl->val = SENSOR_JPEG_SNAPSHOT_MEMSIZE; */
		break;
	}

	case V4L2_CID_CAM_JPEG_QUALITY:
	{
		ctrl->val = state->jpeg_param.quality;
		break;
	}

	case V4L2_CID_CAMERA_OBJ_TRACKING_STATUS:
	{
		break;
	}

	case V4L2_CID_CAMERA_SMART_AUTO_STATUS:
	{
		break;
	}

	case V4L2_CID_CAMERA_AUTO_FOCUS_RESULT:
	{
		if (state->bTouchFocus) {
			if (sensor.get_touch_focus_status == NULL) {
				err = -ENOTSUPP;
				CAM_ERROR_PRINTK(
					"%s %s :get_touch_focus_status is NULL!!!\n",
					sensor.name, __func__);
				break;
			}
			err = sensor.get_touch_focus_status(sd, ctrl);
		} else {
			if (sensor.get_auto_focus_status == NULL) {
				err = -ENOTSUPP;
				CAM_ERROR_PRINTK(
					"%s %s :get_auto_focus_status is NULL!!!\n",
					sensor.name, __func__);
				break;
			}
			err = sensor.get_auto_focus_status(sd, ctrl);
		}

		break;
	}

	case V4L2_CID_CAMERA_AE_STABLE_RESULT:
	{
		err = sensor.get_ae_stable_status(sd, ctrl);
		break;
	}

	case V4L2_CID_CAM_DATE_INFO_YEAR:
	{
		ctrl->val = 2010; /* state->dateinfo.year; */ /*bestiq*/
		break;
	}

	case V4L2_CID_CAM_DATE_INFO_MONTH:
	{
		ctrl->val = 2; /* state->dateinfo.month; */
		break;
	}

	case V4L2_CID_CAM_DATE_INFO_DATE:
	{
		ctrl->val = 25; /* state->dateinfo.date; */
		break;
	}

	case V4L2_CID_CAM_SENSOR_VER:
	{
		ctrl->val = state->camdrv_ss_version;
		break;
	}

	case V4L2_CID_CAM_FW_MINOR_VER:
	{
		ctrl->val = state->fw_ver.minor;
		break;
	}

	case V4L2_CID_CAM_FW_MAJOR_VER:
	{
		ctrl->val = state->fw_ver.major;
		break;
	}

	case V4L2_CID_CAM_PRM_MINOR_VER:
	{
		break;
	}

	case V4L2_CID_CAM_PRM_MAJOR_VER:
	{
		break;
	}

	case V4L2_CID_CAMERA_FLASH_CHECK:
	{
		ctrl->val = state->camera_flash_fire;
		break;
	}

	case V4L2_CID_CAMERA_POSTVIEW_WIDTH:
	{
		ctrl->val = state->postview_info.width;
		break;
	}

	case V4L2_CID_CAMERA_POSTVIEW_HEIGHT:
	{
		ctrl->val = state->postview_info.height;
		break;
	}

	case V4L2_CID_CAMERA_SENSOR_ID:
	{
		/* ctrl->val = SENSOR_ID; */
		break;
	}

	case V4L2_CID_CAMERA_GET_ESD_SHOCK_STATUS:
	{
		if (sensor.getEsdStatus) {
			ctrl->val = sensor.getEsdStatus(sd);
			CAM_INFO_PRINTK("after getEsdStatus : result : %d\n", ctrl->val);
		} else {
			CAM_INFO_PRINTK("%s, getEsdStatus is not implemented in sensor driver !\n", __func__);
			ctrl->val = 0;
		}
		break;
	}
	case V4L2_CID_CAMERA_SCENE_MODE:
	{
		ctrl->val = state->currentScene;
		break;
	}
	default:
	{
		CAM_ERROR_PRINTK("%s %s : default control id =%d failed !!\n", sensor.name, __func__, ctrl->id);
		return -ENOIOCTLCMD;
	}
	}

	return err;
}

static int camdrv_ss_try_ctrl(struct v4l2_ctrl *ctrl)
{
	CAM_INFO_PRINTK("%s:%s : (name=%s, id=%x, type=%d)\n",
		__func__, sensor.name, ctrl->name, ctrl->id, ctrl->type);

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_INITIALIZE:
		ctrl->cur.val = 2;
		break;

	case V4L2_CID_CAM_SET_MODE:
        ctrl->cur.val = INIT_MODE;
        break;

	case V4L2_CID_CAMERA_SCENE_MODE:
        ctrl->cur.val = SCENE_MODE_BASE;
        break;

	case V4L2_CID_CAMERA_FRAME_RATE:
        ctrl->cur.val = FRAME_RATE_BASE;
        break;

	case V4L2_CID_CAMERA_AEAWB_LOCK_UNLOCK:
        ctrl->cur.val = AE_AWB_BASE;
        break;

	case V4L2_CID_CAMERA_WDR:
        ctrl->cur.val = WDR_BASE;
        break;

	case V4L2_CID_CAMERA_TOUCH_AF_AREA:
		/* Initiailized the current of Focus Area value */
		ctrl->cur.val = 0;
		break;

	case V4L2_CID_CAMERA_SET_AUTO_FOCUS:
		ctrl->cur.val = AUTO_FOCUS_BASE;
				break;

	case V4L2_CID_CAMERA_SET_AF_PREFLASH:
        ctrl->cur.val = 0;
        break;

	case V4L2_CID_CAMERA_ANTI_BANDING:
        ctrl->cur.val = ANTI_BANDING_BASE;
        break;
	case V4L2_CID_CAMERA_WHITE_BALANCE:
	  ctrl->cur.val = WHITE_BALANCE_BASE;
        break;
	   case V4L2_CID_CAMERA_METERING:
		  ctrl->cur.val = METERING_BASE;
        break;
	case V4L2_CID_CAMERA_ISO:
		  ctrl->cur.val = ISO_BASE;
        break;
	case V4L2_CID_CAMERA_FOCUS_MODE:
		  ctrl->cur.val = FOCUS_MODE_BASE;
        break;
	case V4L2_CID_CAMERA_BRIGHTNESS:
		  ctrl->cur.val = EV_BASE;
        break;
	case V4L2_CID_CAMERA_EFFECT:
		ctrl->cur.val = IMAGE_EFFECT_BASE;
        break;

	case V4L2_CID_CAMERA_EXIF_SENSOR_INFO:
	{
#if 1
			ctrl->cur.val = 0;  // Initiailized exif val
#else
			struct v4l2_exif_sensor_info exif_info;
			int ret = -1, err;
			struct camdrv_ss_state *state = container_of(ctrl->handler,
					       struct camdrv_ss_state, hdl);

			struct v4l2_subdev *sd = &state->sd;
			ctrl->cur.val = 0;  // Initiailized exif val

			CAM_INFO_PRINTK("%s, V4L2_CID_CAMERA_EXIF_SENSOR_INFO\n", __func__);
			err = camdrv_ss_g_exif_sensor_info(sd, &exif_info);

			ret = copy_to_user(
				(struct v4l2_exif_sensor_info *)ctrl->val,
				&exif_info,
				sizeof(struct v4l2_exif_sensor_info));
#endif
			break;
		}

/* For debugging in the future
	case V4L2_CID_CAMERA_VT_MODE:
	case V4L2_CID_CAMERA_AEAWB_LOCK_UNLOCK:
	case V4L2_CID_CAMERA_FLASH_MODE:
	case V4L2_CID_CAMERA_FLASH_CONTROL:
	case V4L2_CID_CAMERA_BRIGHTNESS:
	case V4L2_CID_CAMERA_WHITE_BALANCE:
	case V4L2_CID_CAMERA_EFFECT:
	case V4L2_CID_CAMERA_ISO:
	case V4L2_CID_CAMERA_METERING:
	case V4L2_CID_CAMERA_CONTRAST:
	case V4L2_CID_CAMERA_SATURATION:
	case V4L2_CID_CAMERA_SHARPNESS:
	case V4L2_CID_CAMERA_WDR:
	case V4L2_CID_CAMERA_ANTI_SHAKE:
	case V4L2_CID_CAMERA_FACE_DETECTION:
	case V4L2_CID_CAMERA_SMART_AUTO:
	case V4L2_CID_CAMERA_FOCUS_MODE:
	case V4L2_CID_CAMERA_DEFAULT_FOCUS_POSITION:
	case V4L2_CID_CAMERA_VINTAGE_MODE:
	case V4L2_CID_CAMERA_BEAUTY_SHOT:
	case V4L2_CID_CAMERA_FACEDETECT_LOCKUNLOCK:
	case V4L2_CID_CAM_JPEG_QUALITY:
	case V4L2_CID_CAMERA_GPS_LATITUDE:
	case V4L2_CID_CAMERA_GPS_LONGITUDE:
	case V4L2_CID_CAMERA_GPS_TIMESTAMP:
	case V4L2_CID_CAMERA_GPS_ALTITUDE:
	case V4L2_CID_CAMERA_ZOOM:
	case V4L2_CID_CAMERA_CAF_START_STOP:
	case V4L2_CID_CAMERA_OBJECT_POSITION_X:
	case V4L2_CID_CAMERA_OBJECT_POSITION_Y:
	case V4L2_CID_CAMERA_OBJ_TRACKING_START_STOP:
	case V4L2_CID_CAMERA_FRAME_RATE:
	case V4L2_CID_CAMERA_ANTI_BANDING:
	case V4L2_CID_CAM_CAPTURE_DONE:
	case V4L2_CID_CAM_PREVIEW_ONOFF:
	case V4L2_CID_CAM_UPDATE_FW:
	case V4L2_CID_CAM_SET_FW_ADDR:
	case V4L2_CID_CAM_SET_FW_SIZE:
	case V4L2_CID_CAM_FW_VER:
	case V4L2_CID_CAMERA_CHECK_DATALINE:
	case V4L2_CID_CAMERA_CHECK_DATALINE_STOP:
	case V4L2_CID_CAMERA_SET_SLOW_AE:
	case V4L2_CID_CAMERA_SET_GAMMA:
	case V4L2_CID_CAMERA_BATCH_REFLECTION:
	case V4L2_CID_CAMERA_ESD_DETECTED_RESTART_CAMERA:
	case V4L2_CID_SENSOR_MOUNT_ORIENTATION:
*/
	default:
			break;
	}
	return 0;
}

static int camdrv_ss_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct camdrv_ss_state *state = container_of(ctrl->handler,
					       struct camdrv_ss_state, hdl);

	struct v4l2_subdev *sd = &state->sd;
	

	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */

	int err = 0;
	int temp_sensor_state = 0;

	//CAM_INFO_PRINTK("%s %s : id = %d\n",
//		sensor.name, __func__, ctrl->id);

	if (ctrl->id == V4L2_CID_CAMERA_INITIALIZE) {

		CAM_INFO_PRINTK("%s : V4L2_CID_CAMERA_INITIALIZE : value  = %d\n", __func__,ctrl->val);
		if(ctrl->val == 2) {
			CAM_INFO_PRINTK("%s : Return  V4L2_CID_CAMERA_INITIALIZE, default val\n", __func__);
			return 0;
		}
		if ((ctrl->val == 1) && (sensor.smartStayChangeInitSetting != NULL)) {
			CAM_INFO_PRINTK("%s: smartStayChangeInitSetting called ..\n", __func__);
			sensor.smartStayChangeInitSetting(&sensor);
		}

		struct task_struct *camdrv_ss_init_thread;
		if (atomic_read(&sensor_state) == CAMDRV_SS_NOT_INITIALIZED) {
			atomic_set(&sensor_state, CAMDRV_SS_INITIALIZING);

			/* Sensor power up is sequential
				function After it is successful
				create seperate thread for init */

			if (0 != camdrv_ss_actual_sensor_power_up()) {
				CAM_ERROR_PRINTK("%s %s :"
				 "camdrv_ss_actual_sensor_power_up"
				  "FAILED. Return ERR !!!!.\n",
				  sensor.name, __func__);
				return -1;
			}
			CAM_INFO_PRINTK("%s %s :creating camera intialize thread...\n", sensor.name, __func__);
			state->current_mode = INIT_MODE;
			state->mode_switch = INIT_DONE_TO_CAMERA_PREVIEW;
			camdrv_ss_init_thread = kthread_run(camdrv_ss_init_thread_func, sd, "camdrv-ss-init-thread-func");
			if (IS_ERR(camdrv_ss_init_thread)) {
				atomic_set(&sensor_state, CAMDRV_SS_INITIALIZE_FAILED);
				CAM_ERROR_PRINTK("%s: sensor_init_thread failed! CAMDRV_SS_INITIALIZE_FAILED !!\n", __func__);
				wake_up_interruptible(&gCamdrvReadyQ);
				return -1;
			}
		}
		CAM_ERROR_PRINTK("%s:Camera already initializing. return\n", __func__);
		return 0;
	}

	if (ctrl->id == V4L2_CID_CAM_SET_MODE) {
		CAM_INFO_PRINTK("%s : V4L2_CID_CAM_SET_MODE : value  = %d\n", __func__,ctrl->val);
		if (state->current_mode == INIT_MODE) {
			if (ctrl->val == CAMERA_PREVIEW_MODE)
				state->mode_switch = INIT_DONE_TO_CAMERA_PREVIEW;
			else if (ctrl->val == CAMCORDER_PREVIEW_MODE)
				state->mode_switch = INIT_DONE_TO_CAMCORDER_PREVIEW;
			else {
				CAM_ERROR_PRINTK("%s: Wrong state setting:current_mode= %d, new_mode =%d  failed !!\n", __func__, state->current_mode, ctrl->val);
				//return	-ENOTSUPP;
			}
		} else if (state->current_mode == CAMERA_PREVIEW_MODE) {
			if (ctrl->val == CAMERA_PREVIEW_MODE)
				CAM_ERROR_PRINTK("%s: CAMERA_PREVIEW_MODE already set !\n", __func__);
				//state->mode_switch = CAMERA_PREVIEW_SIZE_CHANGE;
			else if (ctrl->val == CAMCORDER_PREVIEW_MODE)
				state->mode_switch = CAMERA_PREVIEW_TO_CAMCORDER_PREVIEW;
			else if (ctrl->val == PICTURE_MODE)
				state->mode_switch = CAMERA_PREVIEW_TO_PICTURE_CAPTURE;
			else {
				CAM_ERROR_PRINTK("%s: Wrong state setting:current_mode= %d, new_mode =%d  failed !!\n", __func__, state->current_mode, ctrl->val);
				//return	-ENOTSUPP;
			}
		} else if (state->current_mode == CAMCORDER_PREVIEW_MODE) {
			if (ctrl->val == CAMERA_PREVIEW_MODE)
				state->mode_switch = CAMCORDER_PREVIEW_TO_CAMERA_PREVIEW;
			else if (ctrl->val == CAMCORDER_PREVIEW_MODE)
				CAM_ERROR_PRINTK("%s: CAMCORDER_PREVIEW_MODE already set !\n", __func__);
				//state->mode_switch = CAMCORDER_PREVIEW_SIZE_CHANGE;
			else {
				CAM_ERROR_PRINTK("%s: Wrong state setting:current_mode= %d, new_mode =%d failed !!\n", __func__, state->current_mode, ctrl->val);
				//return	-ENOTSUPP;
			}
		} else if (state->current_mode == PICTURE_MODE) {
			if (ctrl->val == CAMERA_PREVIEW_MODE)
				state->mode_switch = PICTURE_CAPTURE_TO_CAMERA_PREVIEW_RETURN;
			else {
				CAM_ERROR_PRINTK("%s: Wrong state setting:current_mode= %d, new_mode =%d  failed !!\n", __func__, state->current_mode, ctrl->val);
				//return	-ENOTSUPP;
			}
		}

		CAM_ERROR_PRINTK("%s: Mode changed from :current_mode= %d --> new_mode =%d\n", __func__, state->current_mode, ctrl->val);
		state->current_mode = ctrl->val;
		return 0;
	}


	temp_sensor_state = atomic_read(&sensor_state);
	if (temp_sensor_state != CAMDRV_SS_STREAMING && temp_sensor_state != CAMDRV_SS_INITIALIZE_DONE) {
		CAM_ERROR_PRINTK("%s %s :camera not yet initialized : state = %d !!\n", sensor.name, __func__, temp_sensor_state);
		return 0;
	}

	if (state->check_dataline) {
		if ((ctrl->id != V4L2_CID_CAM_PREVIEW_ONOFF) &&
			(ctrl->id != V4L2_CID_CAMERA_CHECK_DATALINE_STOP) &&
			(ctrl->id != V4L2_CID_CAMERA_CHECK_DATALINE)) {
			return 0;
		}
	}

	mutex_lock(&sensor_s_ctrl);

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_VT_MODE:
	{
			err = camdrv_ss_set_vt_mode(sd, ctrl);

		break;
	}

	case V4L2_CID_CAMERA_AEAWB_LOCK_UNLOCK:
	{
		err = camdrv_ss_AE_AWB_lock(sd, ctrl);
		break;
	}

	case V4L2_CID_CAMERA_FLASH_MODE:
	{
	    err = camdrv_ss_set_flash_mode(sd, ctrl);
		break;
	}

	case V4L2_CID_CAMERA_FLASH_CONTROL:
	{
		if (sensor.flash_control != NULL)
			err = sensor.flash_control(sd, ctrl->val);
		break;
	}

	case V4L2_CID_CAMERA_BRIGHTNESS:
	{
		err = camdrv_ss_set_ev(sd, ctrl);
		break;
	}

	case V4L2_CID_CAMERA_WHITE_BALANCE:
	{
		/* err = camdrv_ss_set_white_balance(sd, ctrl); */ /* aska modified */
		if (sensor.set_white_balance != NULL) {
			CAM_ERROR_PRINTK("%s %s :sensor has defined its sequence\n", sensor.name, __func__);
			err = sensor.set_white_balance(sd, ctrl->val);

		} else
			err = camdrv_ss_set_white_balance(sd, ctrl);


		break;
	}

	case V4L2_CID_CAMERA_EFFECT:
	{
		err = camdrv_ss_set_effect(sd, ctrl);
		break;
	}

	case V4L2_CID_CAMERA_ISO:
	{
		/* err = camdrv_ss_set_iso(sd, ctrl); */ /*aska modified */
		err = sensor.set_iso(sd, ctrl->val);
		break;
	}

	case V4L2_CID_CAMERA_METERING:
	{
		err = camdrv_ss_set_metering(sd, ctrl);
		break;
	}

	case V4L2_CID_CAMERA_CONTRAST:
	{
		err = camdrv_ss_set_contrast(sd, ctrl);
		break;
	}

	case V4L2_CID_CAMERA_SATURATION:
	{
		err = camdrv_ss_set_saturation(sd, ctrl);
		break;
	}

	case V4L2_CID_CAMERA_SHARPNESS:
	{
		err = camdrv_ss_set_sharpness(sd, ctrl);
		break;
	}

	case V4L2_CID_CAMERA_WDR:
	{
		/* err = camdrv_ss_set_autocontrast(sd, ctrl); */
		break;
	}

	case V4L2_CID_CAMERA_ANTI_SHAKE:
	{
		break;
	}

	case V4L2_CID_CAMERA_FACE_DETECTION:
	{
		break;
	}

	case V4L2_CID_CAMERA_SMART_AUTO:
	{
		break;
	}

	case V4L2_CID_CAMERA_FOCUS_MODE:
	{
		err = camdrv_ss_set_focus_mode(sd, ctrl->val);
		break;
	}

	case V4L2_CID_CAMERA_DEFAULT_FOCUS_POSITION:
	{
		/* struct i2c_client *client = v4l2_get_subdevdata(sd); */
		/* struct camdrv_ss_state *state = to_state(sd); */
		CAM_INFO_PRINTK("%s %s :  value =%d\n", sensor.name, __func__, state->af_mode);
		err = camdrv_ss_set_focus_mode(sd, state->af_mode);
		break;
	}

	case V4L2_CID_CAMERA_VINTAGE_MODE:
	{
		break;
	}

	case V4L2_CID_CAMERA_BEAUTY_SHOT:
	{
		break;
	}

	case V4L2_CID_CAMERA_FACEDETECT_LOCKUNLOCK:
	{
		break;
	}

	/* need to be modified */
	case V4L2_CID_CAM_JPEG_QUALITY:
	{
		if (ctrl->val < 0 || ctrl->val > 100) {
			err = -EINVAL;
		} else {
			state->jpeg_param.quality = ctrl->val;
			err = camdrv_ss_set_jpeg_quality(sd);
		}
		break;
	}

	case V4L2_CID_CAMERA_SCENE_MODE:
	{
		if (sensor.set_scene_mode != NULL) {  /* denis */
			CAM_ERROR_PRINTK("%s %s set_scene_mode :sensor has defined its sequence\n", sensor.name, __func__);
			sensor.set_scene_mode(sd, ctrl);
		} else
			err = camdrv_ss_set_scene_mode(sd, ctrl);
		break;
	}

	case V4L2_CID_CAMERA_GPS_LATITUDE:
	{
		CAM_ERROR_PRINTK("%s %s : V4L2_CID_CAMERA_GPS_LATITUDE not implemented !!\n", sensor.name, __func__);
		break;
	}

	case V4L2_CID_CAMERA_GPS_LONGITUDE:
	{
		CAM_ERROR_PRINTK("%s %s : V4L2_CID_CAMERA_GPS_LONGITUDE not implemented !!\n", sensor.name, __func__);
		break;
	}

	case V4L2_CID_CAMERA_GPS_TIMESTAMP:
	{
		CAM_ERROR_PRINTK("%s %s : V4L2_CID_CAMERA_GPS_TIMESTAMP not implemented !!\n", sensor.name, __func__);
		break;
	}

	case V4L2_CID_CAMERA_GPS_ALTITUDE:
	{
		CAM_ERROR_PRINTK("%s %s : V4L2_CID_CAMERA_GPS_ALTITUDE not implemented !!\n", sensor.name, __func__);
		break;
	}

	case V4L2_CID_CAMERA_ZOOM:
	{
		err = camdrv_ss_set_dzoom(sd, ctrl);
		CAM_INFO_PRINTK("%s : V4L2_CID_CAMERA_ZOOM success\n", __func__);
		break;
	}

	case V4L2_CID_CAMERA_TOUCH_AF_AREA:
	{
		v4l2_touch_area touch_area;
		int ret = -1;

		ret = copy_from_user(&touch_area, (v4l2_touch_area *)ctrl->val, sizeof(v4l2_touch_area));

		state->touch_area.leftTopX = touch_area.leftTopX;
		state->touch_area.leftTopY = touch_area.leftTopY;
		state->touch_area.rightBottomX = touch_area.rightBottomX;
		state->touch_area.rightBottomY = touch_area.rightBottomY;
		state->touch_area.weight = touch_area.weight;

		if (touch_area.leftTopX == 0 && touch_area.leftTopY == 0 && touch_area.rightBottomX == 1 &&
			touch_area.rightBottomY == 1 && touch_area.weight == 1)
			state->bTouchFocus = false;
		else {
			if (sensor.set_touch_focus_area == NULL) {
				err = -ENOTSUPP;
				CAM_ERROR_PRINTK(
					"%s %s :set_touch_focus_area is NULL!!!\n",
					sensor.name, __func__);
				break;
			}
			sensor.set_touch_focus_area(sd, TOUCH_AF_START, &(state->touch_area));
			state->bTouchFocus = true;
		}

		CAM_INFO_PRINTK(
			"%s : V4L2_CID_CAMERA_TOUCH_AF_AREA  IsTouchFocusAreaValid =%d "
			"x =%d, y =%d ,w = %d, h =%d, weight=%d\n",
			__func__, state->bTouchFocus,
			state->touch_area.leftTopX,
			state->touch_area.leftTopY,
			state->touch_area.rightBottomX,
			state->touch_area.rightBottomY,
			state->touch_area.weight);
		break;
	}

	case V4L2_CID_CAMERA_CAF_START_STOP:
	{
		break;
	}

	case V4L2_CID_CAMERA_OBJECT_POSITION_X:
	{
		state->af_info.x = ctrl->val;
		err = 0;
		break;
	}

	case V4L2_CID_CAMERA_OBJECT_POSITION_Y:
	{
		state->af_info.y = ctrl->val;
		err = 0;
		break;
	}

	case V4L2_CID_CAMERA_OBJ_TRACKING_START_STOP:
	{
		break;
	}

	case V4L2_CID_CAMERA_SET_AUTO_FOCUS:
	{
		CAM_INFO_PRINTK(
			"%s : V4L2_CID_CAMERA_SET_AUTO_FOCUS"
			" value=%d, bTouchFocus=%d, af_mode=%d\n",
			__func__, ctrl->val,
			state->bTouchFocus, state->af_mode);

		if (state->bTouchFocus) {
			if (sensor.set_touch_focus == NULL) {
				err = -ENOTSUPP;
				CAM_ERROR_PRINTK(
					"%s %s :set_touch_focus is NULL!!!\n",
					sensor.name, __func__);
				break;
			}
			if (ctrl->val == AUTO_FOCUS_ON) {
				err = sensor.set_touch_focus(sd, TOUCH_AF_START, NULL);
			}
            /*Backporting Rhea to Hawaii Start: To avoid frame drop in AF cancel, splited currently applied register into three part 
			and pass them one by one
			AUTO_FOCUS_1ST_CANCEL -> pass 1st part of register
			AUTO_FOCUS_2ND_CANCEL -> pass 2nd part of register
			AUTO_FOCUS_OFF -> pass last part
			*/			
			else if(ctrl->val == AUTO_FOCUS_OFF) {
				err = sensor.set_touch_focus(sd, TOUCH_AF_STOP, NULL);
				state->bTouchFocus = false;
			}else if (ctrl->val ==AUTO_FOCUS_1ST_CANCEL)
				{
                                err = sensor.set_touch_focus(sd, AUTO_FOCUS_1ST_CANCEL, NULL);
				}
			       else if (ctrl->val ==AUTO_FOCUS_2ND_CANCEL)
			       	{
                                      err = sensor.set_touch_focus(sd, AUTO_FOCUS_2ND_CANCEL, NULL);
			}
		} else {
			if (sensor.set_auto_focus == NULL) {
				err = -ENOTSUPP;
				CAM_ERROR_PRINTK(
					"%s %s :set_auto_focus is NULL!!!\n",
					sensor.name, __func__);
				break;
			}
			err = sensor.set_auto_focus(sd, ctrl);
		}
		break;
	}

	case V4L2_CID_CAMERA_SET_AF_PREFLASH:
	{
		//err = camdrv_ss_set_af_preflash(sd, ctrl);
		//Backporting Rhea to Hawaii: For Nevis added sensor specific preflash on rotuine
              if(sensor.get_prefalsh_on !=NULL){
                err = sensor.get_prefalsh_on(sd, ctrl);
	      }
	       else{
		err = camdrv_ss_set_af_preflash(sd, ctrl);
	       	}
     
	
		break;
	}

	case V4L2_CID_CAMERA_FRAME_RATE:
	{
		err = camdrv_ss_set_frame_rate(sd, ctrl);
		break;
	}

	case V4L2_CID_CAMERA_ANTI_BANDING:
	{
		break;
	}

	case V4L2_CID_CAM_CAPTURE_DONE:
	{
		err = camdrv_ss_set_capture_done(sd, ctrl);
		break;
	}

	/* Used to start / stop preview operation.
	 * This call can be modified to START/STOP operation, which can be used in image capture also */
	case V4L2_CID_CAM_PREVIEW_ONOFF:
	{
		if (ctrl->val) {
			err = camdrv_ss_set_preview_start(sd);
		} else {
			err = camdrv_ss_set_preview_stop(sd);
		}

		break;
	}

	case V4L2_CID_CAM_UPDATE_FW:
	{
		break;
	}

	case V4L2_CID_CAM_SET_FW_ADDR:
	{
		break;
	}

	case V4L2_CID_CAM_SET_FW_SIZE:
	{
		break;
	}

	case V4L2_CID_CAM_FW_VER:
	{
		break;
	}

	case V4L2_CID_CAMERA_CHECK_DATALINE:
	{
		state->check_dataline = ctrl->val;
		err = 0;
		break;
	}

	case V4L2_CID_CAMERA_CHECK_DATALINE_STOP:
	{
		err = camdrv_ss_set_dataline_onoff(sd, 0);
		break;
	}

	case V4L2_CID_CAMERA_SET_SLOW_AE:
	case V4L2_CID_CAMERA_SET_GAMMA:
	case V4L2_CID_CAMERA_BATCH_REFLECTION:
	{
		break;
	}

	case V4L2_CID_CAMERA_EXIF_SENSOR_INFO:
	{
		struct v4l2_exif_sensor_info exif_info;
		int ret = -1;

		CAM_INFO_PRINTK("%s, V4L2_CID_CAMERA_EXIF_SENSOR_INFO\n", __func__);
		err = camdrv_ss_g_exif_sensor_info(sd, &exif_info);
		ret = copy_to_user(
			(struct v4l2_exif_sensor_info *)ctrl->val,
			&exif_info,
			sizeof(struct v4l2_exif_sensor_info));

		break;
	}
	case V4L2_CID_CAMERA_ESD_DETECTED_RESTART_CAMERA:
	{
		int camera_id = (int)ctrl->val;
		CAM_INFO_PRINTK("%s :V4L2_CID_CAMERA_ESD_DETECTED_RESTART_CAMERA : camera_id = %d\n", __func__, camera_id);
		camdrv_ss_s_stream(sd, 0);
		camdrv_ss_power(camera_id, 0);
		camdrv_ss_power(camera_id, 1);
		camdrv_ss_s_stream(sd, 1);
		break;
	}
	case V4L2_CID_SENSOR_MOUNT_ORIENTATION:
	{
		CAM_INFO_PRINTK("%s :V4L2_CID_GET_SENSOR_MOUNT_ORIENTATION. Dont handle it\n", __func__);
		err = 0;
		break;
	}
	default:
	{
		err = -ENOTSUPP;
		break;
	}
	}

	if (err < 0) {
		CAM_ERROR_PRINTK("%s %s : ERROR for id  =%d but NOT RETURNING Error to App ,treated as WARNING !!!\n", sensor.name, __func__, ctrl->id);
	}

	mutex_unlock(&sensor_s_ctrl);
	/* CAM_INFO_PRINTK("%s %s : %d SUCCESS\n",
			sensor.name, __func__, ctrl->id); */
	return err;
}


static int camdrv_ss_init(struct v4l2_subdev *sd, u32 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;


	CAM_INFO_PRINTK("%s %s :\n", sensor.name, __func__);

	camdrv_ss_init_parameters(sd);

#ifdef CONFIG_LOAD_FILE
	err = camdrv_ss_regs_table_init(sd);
	if (err < 0) {
		CAM_ERROR_PRINTK("%s: camdrv_ss_regs_table_init failed\n", __func__);
		return -ENOIOCTLCMD;
	}
#endif /* CONFIG_LOAD_FILE */
	CAM_INFO_PRINTK("%s %s : rows = %d\n", sensor.name, __func__, sensor.rows_num_init_regs);
	if (sensor.init_regs == NULL)
		CAM_ERROR_PRINTK("%s %s : init_regs is NULL, please check if it is needed !!!\n", sensor.name, __func__);
	else {

			err = camdrv_ss_i2c_set_config_register(client, sensor.init_regs, sensor.rows_num_init_regs, "init_regs");
#ifdef FACTORY_CHECK
			if(camera_antibanding_get() == ANTI_BANDING_60HZ && sensor.antibanding_60hz_regs != NULL) {
				err = camdrv_ss_i2c_set_config_register(client, sensor.antibanding_60hz_regs, sensor.rows_num_antibanding_60hz_regs, "antibanding_60hz_regs");
				CAM_INFO_PRINTK("%s %s : set antibanding 60hz! \n", sensor.name, __func__);
				if (err < 0) {
					CAM_ERROR_PRINTK("%s %s :set antibanding 60hz failed !!\n", sensor.name, __func__);
					return -EINVAL;
				}
			}
#endif
		}
	if (err < 0) {
		CAM_ERROR_PRINTK("%s %s :i2c failed !!\n", sensor.name, __func__);
		return -EINVAL;
	}

	return 0;
}


static int camdrv_ss_cap_mode_change_monitor(struct v4l2_subdev *sd)
{
	struct task_struct *camdrv_ss_cap_mode_change_monitor_thread;
	CAM_INFO_PRINTK("%s %s\n", sensor.name, __func__);

	if (sensor.get_mode_change_reg == NULL) {
		atomic_set(&gCapModeState, CAMDRV_SS_CAPTURE_MODE_NOT_SUPPORT);
		CAM_ERROR_PRINTK(
			"[WARN]%s %s:get_mode_change_reg not support\n",
			sensor.name, __func__);
		return 1;
	}

	atomic_set(&gCapModeState, CAMDRV_SS_CAPTURE_MODE_READ_PROCESSING);
	camdrv_ss_cap_mode_change_monitor_thread =
		kthread_run(camdrv_ss_cap_mode_change_monitor_thread_func,
		sd, "camdrv-ss-capmode-ch-mon-thread");
	if (IS_ERR(camdrv_ss_cap_mode_change_monitor_thread)) {
		atomic_set(&gCapModeState, CAMDRV_SS_CAPTURE_MODE_READ_FAILED);
		CAM_ERROR_PRINTK(
			"%s %s: camdrv_ss_cap_mode_change_monitor_thread failed!\n",
			sensor.name, __func__);
		return -1;
	}

	return 0;
}


static int camdrv_ss_cap_mode_change_monitor_thread_func(void *data)
{
	struct v4l2_subdev *sd = (struct v4l2_subdev *)data;
	enum camdrv_ss_capture_mode_state mode = 0;
	int timeout = CAP_MODE_CHANGE_MORNITOR_TIMEOUT;

	CAM_INFO_PRINTK("%s %s :E\n", sensor.name, __func__);

	do {
		msleep(CAP_MODE_CHANGE_MORNITOR_INTERVAL_MS);
		timeout--;

		mode = sensor.get_mode_change_reg(sd);
		if (mode == CAMDRV_SS_CAPTURE_MODE_READY) {
			atomic_set(&gCapModeState, CAMDRV_SS_CAPTURE_MODE_READY);
		} else if (mode == CAMDRV_SS_CAPTURE_MODE_READ_PROCESSING) {
			atomic_set(&gCapModeState, CAMDRV_SS_CAPTURE_MODE_READ_PROCESSING);
			if (timeout <= 0) {
				atomic_set(&gCapModeState, CAMDRV_SS_CAPTURE_MODE_READ_FAILED);
				CAM_ERROR_PRINTK(
					"[WARN]%s %s:get_mode_change_reg timeout(%d)\n",
					sensor.name, __func__, CAP_MODE_CHANGE_MORNITOR_TIMEOUT);
				break;
			}
		} else {
			atomic_set(&gCapModeState, CAMDRV_SS_CAPTURE_MODE_READ_FAILED);
			CAM_ERROR_PRINTK(
				"[WARN]%s %s:get_mode_change_reg read failed\n",
				sensor.name, __func__);
		}
	} while (mode == CAMDRV_SS_CAPTURE_MODE_READ_PROCESSING);

	CAM_INFO_PRINTK("%s %s :X\n", sensor.name, __func__);
	return 0;
}
/**************************************************************************
 * DRIVER REGISTRATION FACTORS
 ***************************************************************************/


static int camdrv_ss_s_stream(struct v4l2_subdev *sd, int enable)
{
	 struct camdrv_ss_state *state = to_state(sd);
	/* struct i2c_client *client = v4l2_get_subdevdata(sd); */
	int tmp_sensor_state = 0;

	atomic_set(&gCapModeState, CAMDRV_SS_CAPTURE_MODE_NOT_SUPPORT);
	tmp_sensor_state = atomic_read(&sensor_state);

	if (enable && (tmp_sensor_state != CAMDRV_SS_STREAMING)) {
		CAM_INFO_PRINTK("%s %s : START\n", sensor.name, __func__);
		if (tmp_sensor_state == CAMDRV_SS_NOT_INITIALIZED) {
			CAM_ERROR_PRINTK("%s %s : CAMDRV_SS_NOT_INITIALIZED! Initialize again! Normally this case will not happen! !\n", sensor.name, __func__);
			if (0 != camdrv_ss_actual_sensor_power_up()) {
				CAM_ERROR_PRINTK("%s %s :"
					"camdrv_ss_actual_sensor_power_up"
					"FAILED. Return ERR !!!!\n",
					sensor.name, __func__);
				return -1;
			}
			camdrv_ss_init_func(sd);
			tmp_sensor_state = atomic_read(&sensor_state);
		}

		if (tmp_sensor_state == CAMDRV_SS_INITIALIZE_FAILED) {
			CAM_ERROR_PRINTK("%s %s :  CAMDRV_SS_INITIALIZE_FAILED !\n", sensor.name, __func__);
			return -EFAULT;
		} else if (tmp_sensor_state == CAMDRV_SS_INITIALIZING) {
			CAM_ERROR_PRINTK("%s %s : Waiting for the camera initalize thread to finish initialization .........\n", sensor.name, __func__);
			if (0 == wait_event_interruptible_timeout(gCamdrvReadyQ,
				((atomic_read(&sensor_state) == CAMDRV_SS_INITIALIZE_DONE) || (atomic_read(&sensor_state) == CAMDRV_SS_INITIALIZE_FAILED)), msecs_to_jiffies(30000))) { /* aska modify for ivory */
				CAM_ERROR_PRINTK("%s %s : TIMEOUT 6 sec, waited for event CAMDRV_SS_INITIALIZE_DONE: state =%d !\n", sensor.name, __func__, atomic_read(&sensor_state));
				return -EFAULT;
			}
			if (atomic_read(&sensor_state) == CAMDRV_SS_INITIALIZE_FAILED) {
				CAM_ERROR_PRINTK("%s %s :  CAMDRV_SS_INITIALIZE_FAILED happened in init thread ! return ERROR !\n", sensor.name, __func__);
				return -EFAULT;
			}
		} else if (tmp_sensor_state == CAMDRV_SS_INITIALIZE_DONE) {
			CAM_INFO_PRINTK("%s %s : Already initialize is Finished .\n", sensor.name, __func__);
		} else {
			CAM_ERROR_PRINTK("%s %s : WRONG sensor state = %d ! ERROR! FAILED  .\n", sensor.name, __func__, tmp_sensor_state);
			return -EFAULT;
		}

		if (state->mode_switch == CAMERA_PREVIEW_TO_PICTURE_CAPTURE) {
			CAM_ERROR_PRINTK("%s %s : CAPTURE MODE..\n", sensor.name, __func__);

			if (sensor.set_capture_start != NULL) {
				CAM_ERROR_PRINTK("%s %s :sensor has defined its sequence\n", sensor.name, __func__);
				sensor.set_capture_start(sd, NULL);
			} else
				camdrv_ss_set_capture_start(sd, NULL);

			camdrv_ss_cap_mode_change_monitor(sd);
		} else {
			CAM_ERROR_PRINTK("%s %s : CAM or REC PREVIEW MODE mode_switch =%d ..\n", sensor.name, __func__, state->mode_switch);

			/* If the perticular sensor needs a different sequence , it needs be defined in sensor driver file.
			  * otherwise it will be executed in a standard sequence as s5k4ecgx */
			if (state->mode_switch == CAMERA_PREVIEW_TO_CAMCORDER_PREVIEW)
				camdrv_ss_set_preview_stop(sd);

			if (sensor.set_preview_start != NULL) {
				CAM_ERROR_PRINTK("%s %s :sensor has defined its sequence\n", sensor.name, __func__);
				sensor.set_preview_start(sd);
			} else
				camdrv_ss_set_preview_start(sd);
		}

		atomic_set(&sensor_state, CAMDRV_SS_STREAMING);

		CAM_INFO_PRINTK(
			"%s %s : START success\n", sensor.name, __func__);
	}

	if (!enable && (tmp_sensor_state != CAMDRV_SS_INITIALIZE_DONE)) {
		CAM_INFO_PRINTK("%s %s :STOP\n", sensor.name, __func__);
		/* camdrv_ss_set_preview_stop(sd); */
		atomic_set(&sensor_state, CAMDRV_SS_INITIALIZE_DONE);
		CAM_INFO_PRINTK("%s : STOP success\n", __func__);
	}

	return 0;
}

#if 0
static int camdrv_ss_set_bus_param(struct soc_camera_device *icd,
				 unsigned long flags)
{
	/* TODO: Do the right thing here, and validate bus params */
	return 0;
}
static unsigned long camdrv_ss_query_bus_param(struct soc_camera_device *icd)
{
	unsigned long flags = SOCAM_PCLK_SAMPLE_FALLING |
		SOCAM_HSYNC_ACTIVE_HIGH | SOCAM_VSYNC_ACTIVE_HIGH |
		SOCAM_DATA_ACTIVE_HIGH | SOCAM_MASTER;

	/* TODO: Do the right thing here, and validate bus params */

	flags |= SOCAM_DATAWIDTH_10;

	return flags;
}
#endif

static long camdrv_ss_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	return sensor.thumbnail_ioctl(sd, cmd, arg);
}

#if 0
static int camdrv_ss_enum_input(struct soc_camera_device *icd, struct v4l2_input *inp)
{
	struct soc_camera_link *icl = NULL;
	struct v4l2_subdev_sensor_interface_parms *plat_parms;

	if (!icd) {
		CAM_ERROR_PRINTK("%s %s : icd NULL failed !!\n", __func__, sensor.name);
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->std  = V4L2_STD_UNKNOWN;
	if (camera_id == CAMDRV_SS_CAM_ID_MAIN)
		strcpy(inp->name, CAMDRV_SS_MODULE_NAME_MAIN);
	else
		strcpy(inp->name, CAMDRV_SS_MODULE_NAME_SUB);

	if (icl && icl->priv) {
		plat_parms = icl->priv;
		inp->status = 0;
/*Backporting Rhea to Hawaii: Patch from Broadcom for rotation issues CSP:576850*/
		/* Samsung project not use PORTRAIT and LANDSCAPE */
		if (plat_parms->orientation == V4L2_SUBDEV_SENSOR_PORTRAIT) {
			CAM_INFO_PRINTK("%s %s : V4L2_SUBDEV_SENSOR_PORTRAIT and V4L2_SUBDEV_SENSOR_ORIENT_0 are same !\n", __func__, sensor.name);
			plat_parms->orientation = V4L2_SUBDEV_SENSOR_ORIENT_0;
		}
		if (plat_parms->orientation == V4L2_SUBDEV_SENSOR_LANDSCAPE) {
			CAM_INFO_PRINTK("%s %s : V4L2_SUBDEV_SENSOR_LANDSCAPE and V4L2_SUBDEV_SENSOR_ORIENT_90 are same !\n", __func__, sensor.name);
			plat_parms->orientation = V4L2_SUBDEV_SENSOR_ORIENT_90;
		}

		if (plat_parms->orientation ==
			V4L2_SUBDEV_SENSOR_ORIENT_0)
			inp->status &= ~(V4L2_IN_ST_HFLIP | V4L2_IN_ST_VFLIP);
		else if (plat_parms->orientation ==
			V4L2_SUBDEV_SENSOR_ORIENT_90)
			inp->status |= V4L2_IN_ST_HFLIP;
		else if (plat_parms->orientation ==
			V4L2_SUBDEV_SENSOR_ORIENT_180)
			inp->status |= V4L2_IN_ST_VFLIP;
		else if (plat_parms->orientation ==
			V4L2_SUBDEV_SENSOR_ORIENT_270)
			inp->status |= (V4L2_IN_ST_HFLIP | V4L2_IN_ST_VFLIP);

		if (plat_parms->facing == V4L2_SUBDEV_SENSOR_BACK)
			inp->status |= V4L2_IN_ST_BACK;
	}
	return 0;
}
#endif

#if 0
static struct soc_camera_ops camdrv_ss_ops = {
	.set_bus_param		= camdrv_ss_set_bus_param,
	.query_bus_param	= camdrv_ss_query_bus_param,
	.enum_input		= camdrv_ss_enum_input,
	/* .controls			= controls, */
	/* .num_controls		= ARRAY_SIZE(*controls), */
};
#endif


static const struct v4l2_subdev_core_ops camdrv_ss_core_ops = {
	.init		= camdrv_ss_init, /* initializing API */
	/* .s_config	= camdrv_ss_s_config, */ /* Fetch platform data */
	/* .queryctrl	= camdrv_ss_queryctrl, */
	/* .querymenu	= camdrv_ss_querymenu, */
	.g_ctrl		= camdrv_ss_g_ctrl,
	.s_power	= camdrv_ss_subdev_power,
	.s_ctrl		= camdrv_ss_s_ctrl,
	.ioctl		= camdrv_ss_ioctl,
};


static const struct v4l2_subdev_video_ops camdrv_ss_video_ops = {
	.s_stream		= camdrv_ss_s_stream,
	.s_crystal_freq		= camdrv_ss_s_crystal_freq,
	/* .g_fmt		= camdrv_ss_g_fmt, */
	.s_mbus_fmt		= camdrv_ss_s_fmt,
	.enum_framesizes	= camdrv_ss_enum_framesizes,
	.enum_mbus_fsizes	= camdrv_ss_enum_framesizes,
	.enum_frameintervals	= camdrv_ss_enum_frameintervals,
	.enum_mbus_fmt		= camdrv_ss_enum_fmt,
	.try_mbus_fmt		= camdrv_ss_try_fmt,
	.g_parm			= camdrv_ss_g_parm,
	.s_parm			= camdrv_ss_s_parm,
};


static int camdrv_ss_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	CAM_ERROR_PRINTK("%s :sensor wants to skip frames = %d\n", __func__, sensor.skip_frames);
	*frames = sensor.skip_frames;
	return 0;
}


static int camdrv_ss_g_interface_parms(struct v4l2_subdev *sd,
			struct v4l2_subdev_sensor_interface_parms *parms)
{
	struct camdrv_ss_state *state =
		container_of(sd, struct camdrv_ss_state, sd);

	if (!parms)
		return -EINVAL;


	parms->if_type = state->plat_parms->if_type;
	parms->if_mode = state->plat_parms->if_mode;
	parms->parms = state->plat_parms->parms;
	/* parms->parms.serial = mipi_cfgs[ov5640->i_size]; */
	parms->parms.serial.hs_term_time =
		state->plat_parms->parms.serial.hs_term_time;

	if (parms->parms.serial.hs_term_time == 0x0) {
		CAM_ERROR_PRINTK("[!!WARN!!]%s: hs_term_time is default value(%d)\n",
			sensor.name,
			parms->parms.serial.hs_term_time);
	}

	parms->parms.serial.hs_settle_time =
		state->plat_parms->parms.serial.hs_settle_time;
	if (parms->parms.serial.hs_settle_time == 0x0) {
			CAM_ERROR_PRINTK("[!!WARN!!]%s: hs_settle_time is default value(%d)\n",
				sensor.name,
				parms->parms.serial.hs_settle_time);
	}

	return 0;
}


static const struct v4l2_subdev_sensor_ops camdrv_ss_sensor_ops = {
	.g_skip_frames = camdrv_ss_g_skip_frames,
	.g_interface_parms = camdrv_ss_g_interface_parms,
};


static const struct v4l2_subdev_ops camdrv_ss_subdev_ops = {
	.core = &camdrv_ss_core_ops,
	.video = &camdrv_ss_video_ops,
	.sensor = &camdrv_ss_sensor_ops,
};


bool camdrv_ss_power(int cam_id, int bOn)
{
	if (cam_id != CAMDRV_SS_CAM_ID_MAIN && cam_id != CAMDRV_SS_CAM_ID_SUB) {
		CAM_ERROR_PRINTK("%s camera id =%d is WRONG\n", __func__, cam_id);
		return false;
	}
	camera_id = cam_id;

	if (bOn) {
		if (atomic_read(&sensor_state) != CAMDRV_SS_NOT_INITIALIZED) {
			CAM_ERROR_PRINTK(
				"%s:cam_id = %d ,Camera in use by another client\n",
				__func__, cam_id);
			return false;
		}

		memset(&sensor, 0, sizeof(struct camdrv_ss_sensor_cap));
		CAM_INFO_PRINTK("%s : cam_id = %d powering ON ..\n",
				 __func__, cam_id);

#ifdef CONFIG_SOC_MAIN_CAMERA
		if (camera_id == CAMDRV_SS_CAM_ID_MAIN) {
#if 0
			if (camdrv_ss_sensor_init_main == NULL) {
				CAM_INFO_PRINTK("%s: First time main" \
					"camera call. return\n", __func__);
				return true;
			}
#endif

			camdrv_ss_sensor_init_main(&sensor);
			camdrv_ss_generic_flash_control	= sensor.flash_control;
			if (!camdrv_ss_generic_flash_control)
				CAM_ERROR_PRINTK(
						"%s %s : There is no FLASH support\n",
						__func__, sensor.name);

			if ( sensor.rear_camera_vendorid != NULL){
				
				sensor.rear_camera_vendorid(camdrv_rear_camera_vendorid);
				CAM_ERROR_PRINTK(
						"%s  : camdrv_rear_camera_vendorid\n",
						__func__, camdrv_rear_camera_vendorid);
				}
		}
#endif

#ifdef CONFIG_SOC_SUB_CAMERA
		else if (camera_id == CAMDRV_SS_CAM_ID_SUB) {
#if 0
			if (camdrv_ss_sensor_init_sub == NULL) {
				CAM_INFO_PRINTK("%s: First time sub camera" \
					"call. return\n", __func__);
				return true;
			}
#endif
			camdrv_ss_sensor_init_sub(&sensor);
		}
#endif

		if (sensor.thumbnail_ioctl == NULL) {
			CAM_ERROR_PRINTK("%s %s : sensor.thumbnail_ioctl == NULL failed !\n", __func__, sensor.name);
			return false;
		}

		if (sensor.controls == NULL) {
			CAM_ERROR_PRINTK("%s %s : sensor.controls == NULL failed !\n", __func__, sensor.name);
			return false;
		}

		if (sensor.enum_frameintervals == NULL) {
			CAM_ERROR_PRINTK("%s %s : sensor.enum_frameintervals == NULL failed !\n", __func__, sensor.name);
			return false;
		}

		if (sensor.get_exif_sensor_info == NULL) {
			CAM_ERROR_PRINTK(
				"%s %s : sensor.get_exif_sensor_info == NULL failed !\n",
				__func__, sensor.name);
			return false;
		}
#if 0
		camdrv_ss_ops.controls = sensor.controls;
		camdrv_ss_ops.num_controls = sensor.rows_num_controls;
#endif

		CAM_INFO_PRINTK("%s : cam_id = %d power ON. DONE.  [Actually sensor is not powered] !\n", __func__, cam_id);
	} else {
		CAM_INFO_PRINTK("%s : cam_id = %d powering OFF ..\n", __func__, cam_id);
		if (atomic_read(&sensor_state) == CAMDRV_SS_NOT_INITIALIZED) {
			CAM_ERROR_PRINTK("%s %s : Camera already in Power off state! no need to do anything !\n", sensor.name, __func__);
			return true;
		}

		if (atomic_read(&sensor_state) == CAMDRV_SS_INITIALIZING) {
			CAM_ERROR_PRINTK("%s %s : Waiting for the camera initialize to finish and then we will power off camera !\n", sensor.name, __func__);
			if (0 == wait_event_interruptible_timeout(gCamdrvReadyQ, ((atomic_read(&sensor_state) == CAMDRV_SS_INITIALIZE_DONE) || (atomic_read(&sensor_state) == CAMDRV_SS_INITIALIZE_FAILED)), msecs_to_jiffies(6000))) {
				CAM_ERROR_PRINTK("%s %s : Serious problem ,TIMEOUT 6 sec, waited for event INITIALIZE or FAILED: state =%d !\n", sensor.name, __func__, atomic_read(&sensor_state));
			}
			if (atomic_read(&sensor_state) == CAMDRV_SS_INITIALIZE_FAILED)
				CAM_ERROR_PRINTK("%s %s :  waiting finished.  CAMDRV_SS_INITIALIZE_FAILED,Now power off the camera !\n", sensor.name, __func__);
			if (atomic_read(&sensor_state) == CAMDRV_SS_INITIALIZE_DONE)
				CAM_ERROR_PRINTK("%s %s :  waiting finished.  CAMDRV_SS_INITIALIZE_DONE, Now power off the camera !\n", sensor.name, __func__);

		}

		/* For power consumption */
		GPIOSetup.name = PN_BSC1CLK;
		pinmux_get_pin_config(&GPIOSetup);
		GPIOSetup.func = PF_GPIO51;
		GPIOSetup.reg.b.pull_up = 1;
		GPIOSetup.reg.b.pull_dn = 0;
		pinmux_set_pin_config(&GPIOSetup);
		gpio_request(51, "bsc1scl");
		gpio_direction_input(51);
		gpio_free(51);

		GPIOSetup.name = PN_BSC1DAT;
		pinmux_get_pin_config(&GPIOSetup);
		GPIOSetup.func = PF_GPIO52;
		GPIOSetup.reg.b.pull_up = 1;
		GPIOSetup.reg.b.pull_dn = 0;
		pinmux_set_pin_config(&GPIOSetup);
		gpio_request(52, "bsc1sda");
		gpio_direction_input(52);
		gpio_free(52);

		sensor.sensor_power(0);
		atomic_set(&sensor_state , CAMDRV_SS_NOT_INITIALIZED);

		memset(&sensor, 0, sizeof(struct camdrv_ss_sensor_cap));
		CAM_INFO_PRINTK("%s : cam_id = %d powered OFF . Done\n", __func__, cam_id);
	}

	return true;
}

static int camdrv_ss_subdev_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	
	if (!on)
		return soc_camera_power_off(&client->dev, ssdd);

	return soc_camera_power_on(&client->dev, ssdd);

}


/**************************************************************************
 * camdrv_ss_probe
 * Fetching platform data is being done with s_config sd call.
 * In probe routine, we just register sd device
 ***************************************************************************/
static int camdrv_ss_probe(struct i2c_client *client,
								  const struct i2c_device_id *did)
{
	struct camdrv_ss_state *state;
	struct soc_camera_subdev_desc *ssd = client->dev.platform_data;
	//struct soc_camera_desc *sdesc = container_of(ssd, struct soc_camera_desc, subdev_desc);
	//struct v4l2_subdev *subdev = i2c_get_clientdata(client); 
	char device_id[50];
	int ret;
	int i;

	CAM_INFO_PRINTK("%s %s :\n", __func__, sensor.name);

	if (!ssd) {
		dev_err(&client->dev, "camdrv_ss driver needs platform data\n");
		return -EINVAL;
	}

	if (!ssd->drv_priv) {
		dev_err(&client->dev,
			"camdrv_ss driver needs i/f platform data\n");
		return -EINVAL;
	}


	state = kzalloc(sizeof(struct camdrv_ss_state), GFP_KERNEL);
	if (state == NULL) {
		CAM_ERROR_PRINTK("%s %s : alloc failed !!\n", __func__, sensor.name);
		return -ENOMEM;
	}

	state->af_mode = 0;
	state->currentScene = SCENE_MODE_NONE;
	state->currentWB = WHITE_BALANCE_AUTO;
	state->currentMetering = METERING_CENTER;
	state->bStartFineSearch  = false;

	/* strcpy(sd->name, CAMDRV_SS_MODULE_NAME_MAIN); */
//	CAM_INFO_PRINTK("%s %s :\n", __func__, icl->module_name);

	state->plat_parms = ssd->drv_priv;

	/* Registering sd */
	v4l2_i2c_subdev_init(&state->sd, client, &camdrv_ss_subdev_ops);
#if 0
	icd->ops		= &camdrv_ss_ops;
#endif
	if (ssd->flags == CAMDRV_SS_CAM_ID_MAIN)
		strcpy(state->sd.name, CAMDRV_SS_MODULE_NAME_MAIN);
	else
		strcpy(state->sd.name, CAMDRV_SS_MODULE_NAME_SUB);


/* NEED TO FIND OUT THE MOUNTED SENSOR FROM REGISTERED SENSORS */
	if (ssd->flags == CAMDRV_SS_CAM_ID_MAIN) {
		camdrv_ss_sensor_init_main = NULL;

		if (sensors_reg_main_count == 1) {
			for (i = 0; i < sensors_reg_total_count; i++)
				if (sensors_registered[i].isMainSensor) {
					camdrv_ss_sensor_init_main =
					sensors_registered[i].sensor_functions;
					CAM_INFO_PRINTK("%s:" \
					"Mounted main sensor = %s\n",
					__func__, sensors_registered[i].name);
					strcpy(camdrv_ss_main_name,
						sensors_registered[i].name);
					break;
				}
		}
		if (sensors_reg_main_count > 1) {
			for (i = 0; i < sensors_reg_total_count; i++)
				if (sensors_registered[i].isMainSensor) {
					CAM_INFO_PRINTK("%s: " \
					"Checking whether mounted sensor" \
					"is = %s\n", __func__,
					sensors_registered[i].name);
					if ((sensors_registered[i].\
						sensor_power(1)
						>= 0) &&
						(sensors_registered[i].\
						read_device_id(
						client, device_id) >= 0)) {
						CAM_INFO_PRINTK("%s:"\
						"Mounted Main"\
						"sensor name = %s,"\
						"device_id =%s\n",
						__func__,
						sensors_registered[i].name,
						device_id);
						camdrv_ss_sensor_init_main =
						sensors_registered[i].\
						sensor_functions;
						strcpy(camdrv_ss_main_name,
						sensors_registered[i].name);
						sensors_registered[i].\
						sensor_power(0);
						break;
					} else {
						sensors_registered[i].\
						sensor_power(0);
					}
				}
		}
		if (camdrv_ss_sensor_init_main == NULL) {
			CAM_ERROR_PRINTK("%s: No single main camera mounted"\
			"return error\n", __func__);
			return -1;
		} else {
			camdrv_ss_sensor_init_main(&sensor);
		}
	}

	if (ssd->flags == CAMDRV_SS_CAM_ID_SUB) {
		camdrv_ss_sensor_init_sub = NULL;

		if (sensors_reg_sub_count == 1) {
			for (i = 0; i < sensors_reg_total_count; i++)
				if (!sensors_registered[i].isMainSensor) {
					camdrv_ss_sensor_init_sub =
					sensors_registered[i].sensor_functions;
					CAM_INFO_PRINTK("%s: " \
					"Mounted sub sensor = %s\n",
					__func__,
					sensors_registered[i].name);
					strcpy(camdrv_ss_sub_name,
					sensors_registered[i].name);
					break;
				}
		} else if (sensors_reg_sub_count > 1) {
			for (i = 0; i < sensors_reg_total_count; i++)
				if (!sensors_registered[i].isMainSensor) {
					if ((sensors_registered[i].\
						sensor_power(1)
						>= 0) &&
						(sensors_registered[i].\
						read_device_id(client,
						device_id) >= 0)) {

						CAM_INFO_PRINTK("%s: " \
						"Mounted sub"\
						"sensor name = %s, "\
						"device_id =%s\n",
						__func__,
						sensors_registered[i].name,
						device_id);

						camdrv_ss_sensor_init_sub =
						sensors_registered[i].\
						sensor_functions;
						strcpy(camdrv_ss_sub_name,
						sensors_registered[i].\
						name);
						sensors_registered[i].\
						sensor_power(0);
						break;
					} else {
						sensors_registered[i].\
						sensor_power(0);
					}

				}
		}

		if (camdrv_ss_sensor_init_sub == NULL) {
			CAM_ERROR_PRINTK("%s: No single sub camera mounted." \
					"return error.\n", __func__);
			return -1;
		} else {
			camdrv_ss_sensor_init_sub(&sensor);
		}

	}
	ret = camdrv_ss_subdev_power(&state->sd, 1);
	if (ret < 0)
		CAM_ERROR_PRINTK("camdrv_ss_subdev_power power on failed %d\n", ret);


	v4l2_ctrl_handler_init(&state->hdl, sensor.rows_num_controls);
	if (state->hdl.error)
		printk("%s: error set during init itself!\n",__func__);

	const struct v4l2_ctrl_config *controls = sensor.controls;
	for(i=0; i<sensor.rows_num_controls; i++)
	{
		/* register custom control */
		v4l2_ctrl_new_custom(&state->hdl, &controls[i], NULL);
		if (state->hdl.error)
			CAM_ERROR_PRINTK("%s: error v4l2_ctrl_new_custom for i = %d!\n",__func__,i);
	}
			
	ret = v4l2_ctrl_handler_setup(&state->hdl);
	if(ret < 0)
		CAM_ERROR_PRINTK("%s: Failed to setup control handler\n", __func__);

	state->sd.ctrl_handler = &state->hdl;


#ifdef FACTORY_CHECK
	if (cam_class_init == false) {
		CAM_INFO_PRINTK("Start create class for factory test mode !\n");
		camera_class = class_create(THIS_MODULE, "camera");
	}

	{
		if (camera_class) {
			if (camera_id == CAMDRV_SS_CAM_ID_MAIN) {
				CAM_INFO_PRINTK("Creat Main cam device !\n");

				sec_main_cam_dev = device_create(camera_class, NULL, 0, NULL, "rear");

				if (IS_ERR(sec_main_cam_dev)) {
					CAM_ERROR_PRINTK("Failed to create device(sec_main_cam_dev)!\n");
				}
				if (device_create_file(sec_main_cam_dev, &dev_attr_rear_camtype) < 0) {
					CAM_ERROR_PRINTK("failed to create main camera device file, %s\n",
					dev_attr_rear_camtype.attr.name);
				}
				if (device_create_file(sec_main_cam_dev, &dev_attr_rear_camfw) < 0) {
					CAM_ERROR_PRINTK("failed to create main camera device file, %s\n",
					dev_attr_rear_camfw.attr.name);
				}

			if (device_create_file(sec_main_cam_dev, &camera_antibanding_attr) < 0) {
				CAM_ERROR_PRINTK("Failed to create anti-banding device file,  %s\n",
				camera_antibanding_attr.attr.name);
			}
                 /* AT command flash implementation */
                  /*Backporting Rhea to Hawaii start:*/
	          #ifdef CONFIG_FLASH_ENABLE
			if (device_create_file(sec_main_cam_dev, &dev_attr_rear_flash) < 0) {
			CAM_INFO_PRINTK( "[%s]: failed to create device file, %s\n", __func__,dev_attr_rear_flash.attr.name);
			return 0;
			}
                  #endif
				   if (device_create_file(sec_main_cam_dev, &dev_attr_rear_vendorid) < 0) 
			{
			CAM_INFO_PRINTK( "[%s]: failed to create device file, %s\n", __func__,dev_attr_rear_vendorid.attr.name);
			return 0;
			}
                  /*Backporting Rhea to Hawaii End */
			} else {
				CAM_ERROR_PRINTK("Creat Sub cam device !\n");

				sec_sub_cam_dev = device_create(camera_class, NULL, 1, NULL, "front");
				if (IS_ERR(sec_sub_cam_dev))	{
					CAM_ERROR_PRINTK("Failed to create device(sec_sub_cam_dev)!\n");
				}
				if (device_create_file(sec_sub_cam_dev, &dev_attr_front_camtype) < 0) {
					CAM_ERROR_PRINTK("failed to create sub camera device file, %s\n",
					dev_attr_front_camtype.attr.name);
				}
				if (device_create_file(sec_sub_cam_dev, &dev_attr_front_camfw) < 0) {
					CAM_ERROR_PRINTK("failed to create sub camera device file, %s\n",
					dev_attr_front_camfw.attr.name);
				}
			}
		}

		cam_class_init = true;
	}
#endif

	/* For power consumption */
	GPIOSetup.name = PN_BSC1CLK;
	pinmux_get_pin_config(&GPIOSetup);
	GPIOSetup.func = PF_GPIO51;
	GPIOSetup.reg.b.pull_up = 1;
	GPIOSetup.reg.b.pull_dn = 0;
	pinmux_set_pin_config(&GPIOSetup);
	gpio_request(51, "bsc1scl");
	gpio_direction_input(51);
	gpio_free(51);

	GPIOSetup.name = PN_BSC1DAT;
	pinmux_get_pin_config(&GPIOSetup);
	GPIOSetup.func = PF_GPIO52;
	GPIOSetup.reg.b.pull_up = 1;
	GPIOSetup.reg.b.pull_dn = 0;
	pinmux_set_pin_config(&GPIOSetup);
	gpio_request(52, "bsc1sda");
	gpio_direction_input(52);
	gpio_free(52);

	CAM_INFO_PRINTK("%s %s : success\n", sensor.name, __func__);

	ret = camdrv_ss_subdev_power(&state->sd, 0);
	if (ret < 0)
		printk("camdrv_ss_subdev_power power off failed %d\n", ret);

	return 0;
}


/**************************************************************************
 * camdrv_ss_remove
 ***************************************************************************/
static int camdrv_ss_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	 struct camdrv_ss_state *state = to_state(sd);
	/* struct soc_camera_device *icd = client->dev.platform_data; */
	/* del struct soc_camera_link *icl = to_soc_camera_link(icd); */

	CAM_INFO_PRINTK("%s %s :\n", sensor.name, __func__);

#ifdef CONFIG_LOAD_FILE
	camdrv_ss_regs_table_exit();
#endif /* CONFIG_LOAD_FILE */

	v4l2_device_unregister_subdev(sd);
	v4l2_ctrl_handler_free(&state->hdl);

	kfree(to_state(sd));

#ifdef FACTORY_CHECK__
			cam_class_init = false;
#endif

 #ifdef FACTORY_CHECK______CYK
		if (sec_cam_dev != NUL) {
			device_remove_file(sec_main_cam_dev, &dev_attr_camtype);

			device_destroy(camera, 0);
	}
#endif

	CAM_INFO_PRINTK("%s %s : unloaded\n", sensor.name, __func__);

	return 0;
}


static const struct i2c_device_id camdrv_ss_id_sub[] = {
	{ CAMDRV_SS_MODULE_NAME_SUB, 1 },
};


static struct i2c_driver camdrv_ss_i2c_driver_sub = {
	.driver = {
		.name = CAMDRV_SS_MODULE_NAME_SUB,
	},

	.probe = camdrv_ss_probe,
	.remove = camdrv_ss_remove,
	.id_table = camdrv_ss_id_sub,
};


static const struct i2c_device_id camdrv_ss_id[] = {
	{ CAMDRV_SS_MODULE_NAME_MAIN, 0 },
};


MODULE_DEVICE_TABLE(i2c, camdrv_ss_id);

static struct i2c_driver camdrv_ss_i2c_driver = {
	.driver = {
		.name = CAMDRV_SS_MODULE_NAME_MAIN,
	},

	.probe = camdrv_ss_probe,
	.remove = camdrv_ss_remove,
	.id_table = camdrv_ss_id,
};


static int __init camdrv_ss_mod_init(void)
{
	int err;
	CAM_INFO_PRINTK("%s\n", __func__);

#ifdef CONFIG_SOC_SUB_CAMERA
	err = i2c_add_driver(&camdrv_ss_i2c_driver_sub);
	if(err) {
		CAM_INFO_PRINTK("%s main camera i2c add failed\n", __func__);
		return err;
	}
#endif

#ifdef CONFIG_SOC_MAIN_CAMERA
	err = i2c_add_driver(&camdrv_ss_i2c_driver);
	if(err) {
		CAM_INFO_PRINTK("%s main camera i2c add failed\n", __func__);
		return err;
	}
#endif

	return 0;
}

static void __exit camdrv_ss_mod_exit(void)
{
#ifdef CONFIG_SOC_SUB_CAMERA
	i2c_del_driver(&camdrv_ss_i2c_driver_sub);
#endif

#ifdef CONFIG_SOC_MAIN_CAMERA
	i2c_del_driver(&camdrv_ss_i2c_driver);
#endif
}

int camdrv_ss_sensors_register(struct camdrv_ss_sensor_reg *sens)
{
	sensors_registered[sensors_reg_total_count].sensor_functions
			= sens->sensor_functions;
	sensors_registered[sensors_reg_total_count].sensor_power
			= sens->sensor_power;
	sensors_registered[sensors_reg_total_count].read_device_id
			= sens->read_device_id;
	sensors_registered[sensors_reg_total_count].isMainSensor
			= sens->isMainSensor;
	strncpy(sensors_registered[sensors_reg_total_count].name,
		sens->name, sizeof(sens->name));

	if (sens->isMainSensor)	{
		CAM_INFO_PRINTK("%s: Main sensor registered. Name = %s\n",
				__func__, sens->name);
		sensors_reg_main_count++;
	} else {
		CAM_INFO_PRINTK("%s: Sub sensor registered. Name = %s\n",
				__func__, sens->name);
		sensors_reg_sub_count++;
	}

	sensors_reg_total_count++;
	return 0;
}
EXPORT_SYMBOL(camdrv_ss_sensors_register);

late_initcall(camdrv_ss_mod_init);
module_exit(camdrv_ss_mod_exit);


MODULE_DEVICE_TABLE(i2c, camdrv_ss_id);
MODULE_DESCRIPTION("SAMSUNG CAMERA SENSOR DRIVER ");
MODULE_AUTHOR("Samsung");
MODULE_LICENSE("GPL");

