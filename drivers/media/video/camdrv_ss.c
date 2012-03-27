/***********************************************************************
* Driver for S5K5CCGX (3MP Camera) from SAMSUNG SYSTEM LSI
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
***********************************************************************/
#include <linux/i2c.h>
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

extern bool camdrv_ss_sensor_init_main(bool bOn, struct camdrv_ss_sensor_cap *sensor);

#ifdef CONFIG_SOC_SUB_CAMERA
extern bool camdrv_ss_sensor_init_sub(bool bOn, struct camdrv_ss_sensor_cap *sensor);
#endif

/* #define CONFIG_LOAD_FILE */

#define CAMDRV_SS_MODULE_NAME_MAIN		"camdrv_ss"
#define CAMDRV_SS_MODULE_NAME_SUB		"camdrv_ss_sub"

#define CAMDRV_SS_DEBUG
/* #define FACTORY_CHECK 1 */
static int camera_id = 0;

#ifdef CAMDRV_SS_DEBUG
#define CAM_ERROR_MSG(dev, format, arg...) printk(format, ## arg) /* dev_err(dev, format, ## arg) */
#define CAM_WARN_MSG(dev, format, arg...)  printk(format, ## arg) /* dev_warn(dev, format, ## arg) */
#define CAM_INFO_MSG(dev, format, arg...)  printk(format, ## arg) /* dev_warn(dev, format, ## arg) */
#define CAM_PRINTK(format, arg...)          printk(format, ## arg)
#else
#define CAM_ERROR_MSG(dev, format, arg...)  dev_err(dev, format, ## arg)
#define CAM_WARN_MSG(dev, format, arg...)   dev_warn(dev, format, ## arg)
#define CAM_INFO_MSG(dev, format, arg...)
#define CAM_PRINTK(format, arg...)
#endif /* camdrv_ss_DEBUG */


static bool bCameraInitialized = false;
static bool bCameraPowerUp = false;
static bool bCaptureMode = false;

/* Camera register settings */
static struct camdrv_ss_sensor_cap   sensor;

#define CAMDRV_SS_CAM_ID_MAIN 0
#define CAMDRV_SS_CAM_ID_SUB 1

enum camdrv_ss_operation_mode {
	CAMDRV_SS_OP_MODE_VIDEO = 0,
	CAMDRV_SS_OP_MODE_IMAGE = 1,
};

enum  camdrv_ss_running_mode {
	CAMDRV_SS_RUNNING_MODE_NOTREADY,
	CAMDRV_SS_RUNNING_MODE_IDLE,
	CAMDRV_SS_RUNNING_MODE_RUNNING,
	CAMDRV_SS_RUNNING_MODE_CAPTURE,
};


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
};

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

struct camdrv_ss_state {
	struct camera_platform_data *platform_data;
	struct v4l2_subdev sd;
	struct v4l2_pix_format pix;
	struct v4l2_fract time_per_frame;
	struct camdrv_ss_userset userset;
	struct camdrv_jpeg_param jpeg_param;
	struct v4l2_subdev_sensor_interface_parms *plat_parms;
	struct camdrv_ss_version fw_ver;
	struct camdrv_ss_af_info af_info;
	struct camdrv_ss_image_info postview_info;
	struct v4l2_streamparm strm;
	enum camdrv_ss_running_mode runmode;
	enum camdrv_ss_operation_mode op_mode;
	int sensor_mode;
	int capture_framesize_index;
	int preview_framesize_index;
	int camdrv_ss_version;
	int mclk_freq; /* MCLK in Hz */
	int fps;
	int capture_mode;
	int check_dataline;
	int current_flash_mode;
	int camera_flash_fire;
	int camera_af_flash_fire;
	int camera_af_flash_checked;

	int af_mode;
	int currentScene;
	int currentWB;
	int currentMetering;
	int bStartFineSearch;
	int isoSpeedRating;
	int exposureTime;
	v4l2_touch_area touch_area;
	bool bTouchFocus ;
};


/**************************************************************************
* GLOBAL, STATIC VARIABLES
***************************************************************************/
extern unsigned int HWREV;

/* protect s_ctrl calls */
static DEFINE_MUTEX(sensor_s_ctrl);
static DEFINE_MUTEX(af_cancel_op);

static int camdrv_ss_find_preview_framesize(u32 width, u32 height);
static int camdrv_ss_find_capture_framesize(u32 width, u32 height);



#ifdef FACTORY_CHECK
static bool camtype_init = false;
ssize_t camtype_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("%s \n", __func__);
	char *sensorname = "SAMSUNG_CAMDRV_SS_NONE";

	return sprintf(buf, "%s\n", sensorname);
}

ssize_t camtype_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	printk(KERN_NOTICE "%s:%s\n", __func__, buf);

	return size;
}

static DEVICE_ATTR(camtype, 0644, camtype_show, camtype_store);

extern struct class *sec_class;
struct device *sec_cam_dev = NULL;

#endif


/**************************************************************************
* EXTERN VARIABLES
***************************************************************************/

/**************************************************************************
* FUNCTION DECLARE
***************************************************************************/


static inline struct camdrv_ss_state *to_state(struct v4l2_subdev *sd)
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
#include <asm/uaccess.h>

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
	{"init_regs",				NULL},
	{"preview_camera_regs",			NULL},
	{"snapshot_normal_regs",		NULL},
	{"snapshot_lowlight_regs",		NULL},
	{"snapshot_highlight_regs",		NULL},
	{"snapshot_nightmode_regs",		NULL},
	{"snapshot_flash_on_regs",		NULL},
	{"snapshot_af_preflash_on_regs",	NULL},
	{"snapshot_af_preflash_off_regs",	NULL},
	{"af_macro_mode_regs",			NULL},
	{"af_normal_mode_regs",			NULL},
	{"single_af_start_regs",		NULL},
	{"get_1st_af_search_status",		NULL},
	{"get_2nd_af_search_status",		NULL},
	{"single_af_stop_regs",			NULL},
	{"effect_normal_regs",			NULL},
	{"effect_negative_regs",		NULL},
	{"effect_sepia_regs",			NULL},
	{"effect_mono_regs",			NULL},
	{"effect_aqua_regs",			NULL},
	{"effect_sharpen_regs",			NULL},
	{"effect_solarization_regs",		NULL},
	{"effect_black_white_regs",		NULL},
	{"wb_auto_regs",			NULL},
	{"wb_sunny_regs",			NULL},
	{"wb_cloudy_regs",			NULL},
	{"wb_tungsten_regs",			NULL},
	{"wb_fluorescent_regs",			NULL},
	{"wb_cwf_regs",				NULL},
	{"metering_matrix_regs",		NULL},
	{"metering_center_regs",		NULL},
	{"metering_spot_regs",			NULL},
	{"ev_minus_4_regs",			NULL},
	{"ev_minus_3_regs",			NULL},
	{"ev_minus_2_regs",			NULL},
	{"ev_minus_1_regs",			NULL},
	{"ev_default_regs",			NULL},
	{"ev_plus_1_regs",			NULL},
	{"ev_plus_2_regs",			NULL},
	{"ev_plus_3_regs",			NULL},
	{"ev_plus_4_regs",			NULL},
	{"contrast_minus_2_regs",		NULL},
	{"contrast_minus_1_regs",		NULL},
	{"contrast_default_regs",		NULL},
	{"contrast_plus_1_regs",		NULL},
	{"contrast_plus_2_regs",		NULL},
	{"sharpness_minus_3_regs",		NULL},
	{"sharpness_minus_2_regs",		NULL},
	{"sharpness_minus_1_regs",		NULL},
	{"sharpness_default_regs",		NULL},
	{"sharpness_plus_1_regs",		NULL},
	{"sharpness_plus_2_regs",		NULL},
	{"sharpness_plus_3_regs",		NULL},
	{"saturation_minus_2_regs",		NULL},
	{"saturation_minus_1_regs",		NULL},
	{"saturation_default_regs",		NULL},
	{"saturation_plus_1_regs",		NULL},
	{"saturation_plus_2_regs",		NULL},
	{"zoom_00_regs",			NULL},
	{"zoom_01_regs",			NULL},
	{"zoom_02_regs",			NULL},
	{"zoom_03_regs",			NULL},
	{"zoom_04_regs",			NULL},
	{"zoom_05_regs",			NULL},
	{"zoom_06_regs",			NULL},
	{"zoom_07_regs",			NULL},
	{"zoom_08_regs",			NULL},
	{"scene_none_regs",			NULL},
	{"scene_portrait_regs",			NULL},
	{"scene_nightshot_regs",		NULL},
	{"scene_backlight_regs",		NULL},
	{"scene_landscape_regs",		NULL},
	{"scene_sports_regs",			NULL},
	{"scene_party_indoor_regs",		NULL},
	{"scene_beach_snow_regs",		NULL},
	{"scene_sunset_regs",			NULL},
	{"scene_duskdawn_regs",			NULL},
	{"scene_fall_color_regs",		NULL},
	{"scene_fireworks_regs",		NULL},
	{"scene_candle_light_regs",		NULL},
	{"scene_text_regs",			NULL},
	{"fps_auto_regs",			NULL},
	{"fps_5_regs",				NULL},
	{"fps_7_regs",				NULL},
	{"fps_10_regs",				NULL},
	{"fps_15_regs",				NULL},
	{"fps_20_regs",				NULL},
	{"fps_25_regs",				NULL},
	{"fps_30_regs",				NULL},
	{"fps_60_regs",				NULL},
	{"fps_120_regs",			NULL},
	{"quality_superfine_regs",		NULL},
	{"quality_fine_regs",			NULL},
	{"quality_normal_regs",			NULL},
	{"quality_economy_regs",		NULL},
	{"preview_size_176x144_regs",		NULL},
	{"preview_size_320x240_regs",		NULL},
	{"preview_size_352x288_regs",		NULL},
	{"preview_size_640x480_regs",		NULL},
	{"preview_size_704x576_regs",		NULL},
	{"preview_size_720x480_regs",		NULL},
	{"preview_size_800x480_regs",		NULL},
	{"preview_size_800x600_regs",		NULL},
	{"preview_size_1024x600_regs",		NULL},
	{"preview_size_1024x768_regs",		NULL},
	{"preview_size_1280x960_regs",		NULL},
	{"preview_size_1600x960_regs",		NULL},
	{"preview_size_1600x1200_regs",		NULL},
	{"preview_size_2048x1232_regs",		NULL},
	{"preview_size_2048x1536_regs",		NULL},
	{"preview_size_2560x1920_regs",		NULL},
	{"capture_size_640x480_regs",		NULL},
	{"capture_size_720x480_regs",		NULL},
	{"capture_size_800x480_regs",		NULL},
	{"capture_size_800x486_regs",		NULL},
	{"capture_size_800x600_regs",		NULL},
	{"capture_size_1024x600_regs",		NULL},
	{"capture_size_1024x768_regs",		NULL},
	{"capture_size_1280x960_regs",		NULL},
	{"capture_size_1600x960_regs",		NULL},
	{"capture_size_1600x1200_regs",		NULL},
	{"capture_size_2048x1232_regs",		NULL},
	{"capture_size_2048x1536_regs",		NULL},
	{"capture_size_2560x1536_regs",		NULL},
	{"capture_size_2560x1920_regs",		NULL},
	{"pattern_on_regs",			NULL},
	{"pattern_off_regs",			NULL},
	{"ae_lock_regs",			NULL},
	{"ae_unlock_regs",			NULL},
	{"awb_lock_regs",			NULL},
	{"awb_unlock_regs",			NULL}
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
	char reg_buf[7], data_buf[7];
	int reg_index = 0;

	reg_buf[6] = '\0';
	data_buf[6] = '\0';

	while (camdrv_ss_regs_get_line(current_line)) {
		if (camdrv_ss_regs_trim(current_line) == false) {
			continue;
		}

		/* Check End line of a table. */
		if ((current_line[0] == '}') && (current_line[1] == ';')) {
			break;
		}

		/* Parsing a register format : {0x0000, 0x0000}, */
		if ((current_line[0] == '{') && (current_line[1] == '0') && (current_line[15] == '}')) {
			memcpy(reg_buf, (const void *)&current_line[1], 6);
			memcpy(data_buf, (const void *)&current_line[9], 6);

			reg_table[reg_index].subaddr = (unsigned short)simple_strtoul(reg_buf, NULL, 16);
			reg_table[reg_index].value = (unsigned int)simple_strtoul(data_buf, NULL, 16);

			reg_index++;
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

	if (bFound_table) {
		reg_num_of_element = camdrv_ss_regs_parse_table();
	} else {
		CAM_ERROR_MSG(&client->dev, "[%s: %d] %s reg_table doesn't exist\n", __FILE__, __LINE__, name);
		return -EIO;
	}

	err = camdrv_ss_i2c_set_config_register(client, reg_table, reg_num_of_element, name);
	if (err < 0) {
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! camdrv_ss_i2c_set_data_burst failed\n", __FILE__, __LINE__);
		return -EIO;
	}

	return err;
}


#define CAMDRV_SS_TUNING_FILE_PATH	"/mnt/sdcard/camtuning/Camdrv_ss_%s.h"
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

	CAM_INFO_MSG(&client->dev, "%s %d \n", __func__ , __LINE__);

	memset(TUNING_FILE_PATH, 0x00, CAMDRV_SS_MAX_PATH+1);
	sprintf(TUNING_FILE_PATH, CAMDRV_SS_TUNING_FILE_PATH, sensor.name);
	CAM_PRINTK("%s %d, PATH = %s, sensor name = %s\n", __func__, __LINE__, TUNING_FILE_PATH, sensor.name);

	set_fs(get_ds());

	if (TUNING_FILE_PATH == NULL) {
		CAM_INFO_MSG(&client->dev, "TUNING FILE PATH is NULL!!! %s %d\n", __func__, __LINE__);
		return -EIO;
	}

	filp = filp_open(TUNING_FILE_PATH, O_RDONLY, 0);

	if (IS_ERR(filp)) {
		CAM_PRINTK(KERN_ERR "file open error\n");
		return -EIO;
	}

	l = filp->f_path.dentry->d_inode->i_size;
	CAM_PRINTK("%s file size = %ld\n", __func__, l);

	msleep(50);

	CAM_PRINTK("%s %d\n", __func__, __LINE__);

	for (retry_cnt = 5; retry_cnt > 0; retry_cnt--) {
		dp = vmalloc(l);

		if (dp != NULL) {
			break;
		}

		msleep(50);
	}

	if (dp == NULL) {
		CAM_PRINTK(KERN_ERR "Out of Memory\n");
		filp_close(filp, current->files);
		return -ENOMEM;
	}

	memset(dp, 0, l);

	pos = 0;
	ret = vfs_read(filp, (char __user *)dp, l, &pos);

	if (ret != l) {
		CAM_PRINTK(KERN_ERR "Failed to read file ret = %d\n", ret);
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

		CAM_PRINTK("denis :::  %s %d, count = %d\n", __func__, __LINE__, i);

		while (camdrv_ss_regs_get_line(current_line)) {
			if (strstr(current_line, reg_hash_table[i].name) != NULL) {
				bFound_name = true;
				reg_hash_table[i].location_ptr = location_ptr;
				break;
			}

			location_ptr = curr_pos_ptr;
		}

		if (bFound_name == false) {
			if (i == 0) {
				CAM_PRINTK(KERN_ERR "[%s : %d] ERROR! Couldn't find the reg name in hash table\n", __FILE__, __LINE__);
				return -EIO;
			} else {
				curr_pos_ptr = reg_hash_table[i-1].location_ptr;
			}
			location_ptr = curr_pos_ptr;

			CAM_PRINTK(KERN_ERR "[%s : %d] ERROR! Couldn't find the reg name in hash table\n", __FILE__, __LINE__);
		}
	}

	CAM_PRINTK("camdrv_ss_reg_table_init Done!\n");

	return 0;
}


void camdrv_ss_regs_table_exit(void)
{
	CAM_PRINTK("%s start\n", __func__);

	if (regs_buf_ptr) {
		vfree(regs_buf_ptr);
		regs_buf_ptr = NULL;
	}

	CAM_PRINTK("%s done\n", __func__);
}
#endif /* CONFIG_LOAD_FILE */


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
 int camdrv_ss_i2c_read_4_bytes(struct i2c_client *client,
				unsigned short subaddr,
				unsigned short *data)
{
	unsigned char buf[4];
	struct i2c_msg msg = {client->addr, 0, 2, buf};
	int err = 0;

	if (!client->adapter) {
		CAM_ERROR_MSG(&client->dev, "%s %s : client->adapter = NULL!!! \n", sensor.name, __func__);
		return -EIO;
	}

	buf[0] = subaddr >> 8;
	buf[1] = subaddr & 0xff;

	err = i2c_transfer(client->adapter, &msg, 1);
	if (err < 0) {
		CAM_ERROR_MSG(&client->dev, "%s %s :i2c transfer failed at address %d ! \n", sensor.name, __func__, subaddr);
		return -EIO;
	}

	msg.flags = I2C_M_RD;
	msg.len = 2;

	err = i2c_transfer(client->adapter, &msg, 1);
	if (err < 0) {
		CAM_ERROR_MSG(&client->dev, "%s %s :i2c transfer failed at address %d ! \n", sensor.name, __func__, subaddr);
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
		CAM_ERROR_MSG(&client->dev, "%s %s : client->adapter = NULL ! \n", sensor.name, __func__);
		return -EIO;
	}
	/* CAM_ERROR_MSG(&client->dev, "%s  :i2c transfer1 0x%x   %x\n", __func__, subaddr, data); */

	buf[0] = subaddr >> 8;
	buf[1] = subaddr & 0xFF;
	buf[2] = data >> 8;
	buf[3] = data & 0xFF;


	err = i2c_transfer(client->adapter, &msg, 1);

	/* CAM_ERROR_MSG(&client->dev, "%s  :i2c transfer2 0x%x  %x\n", __func__, subaddr, data); */

	if (err < 0) {
		CAM_ERROR_MSG(&client->dev, "%s %s :i2c transfer failed at address %d ! \n", sensor.name, __func__, subaddr);
		return -EIO;
	}

	return 0;
}


int camdrv_ss_i2c_write_2_bytes(struct i2c_client *client,
				unsigned short data)
{
	int retry_count = 3;
	unsigned char buf[2];
	struct i2c_msg msg = {client->addr, 0, 2, buf};
	int err = 0;

	if (!client->adapter) {
		CAM_ERROR_MSG(&client->dev, "%s %s : client->adapter = NULL ! \n", sensor.name, __func__);
		return -EIO;
	}

	buf[0] = data >> 8;
	buf[1] = data & 0xff;


	err = i2c_transfer(client->adapter, &msg, 1);
	/* CAM_ERROR_MSG(&client->dev, "%s %s :i2c transfer 0x%x \n", __func__, data); */

	if (err < 0) {
		CAM_ERROR_MSG(&client->dev, "%s %s :i2c transfer failed at address %d ! \n", sensor.name, __func__, data);
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
	int i = 0;
	unsigned short subaddr = 0, data_value = 0;

#ifdef CONFIG_LOAD_FILE
	err = camdrv_ss_regs_table_write(client, name);
#else

	if (sensor.i2c_set_data_burst != NULL) {
		err = sensor.i2c_set_data_burst(client, reg_buffer, num_of_regs);
	} else {
		for (i = 0; i < num_of_regs; i++) {
			if (sensor.register_size == 4) {
				subaddr = (reg_buffer[i] >> 16);
				data_value = reg_buffer[i];
				err = camdrv_ss_i2c_write_4_bytes(client, subaddr, data_value);
			} else if (sensor.register_size == 2) {
				subaddr = 0;
				data_value = reg_buffer[i];
				err = camdrv_ss_i2c_write_2_bytes(client, data_value);
			} else {
				CAM_ERROR_MSG(&client->dev, "%s %s :register size is not 4 or 2 bytes = %d ! \n", sensor.name, __func__, sensor.register_size);
				return -EIO;
			}

			if (err < 0) {
				CAM_ERROR_MSG(&client->dev, "%s %s :i2c transfer failed ! \n", sensor.name, __func__);
				return -EIO;
			}
		}
	}

#endif /* CONFIG_LOAD_FILE */

	return err;
}

static int camdrv_ss_set_flash_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{

	struct camdrv_ss_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	CAM_INFO_MSG(&client->dev, "%s %s :camdrv_ss_set_flash_mode E \n", sensor.name, __func__);

	switch (ctrl->value) {
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
		if (sensor.AAT_flash_control != NULL)
			sensor.AAT_flash_control(sd, FLASH_CONTROL_LOW_LEVEL);
		else
			CAM_ERROR_MSG(&client->dev, "%s %s :AAT_flash_control is NULL!!!s \n", sensor.name, __func__);

		break;
	}

	case FLASH_MODE_TORCH_OFF:
	{
		if (sensor.AAT_flash_control != NULL)
			sensor.AAT_flash_control(sd, FLASH_CONTROL_OFF);
		else
			CAM_ERROR_MSG(&client->dev, "%s %s :AAT_flash_control is NULL!!!s \n", sensor.name, __func__);

		break;
	}

	case FLASH_MODE_OFF:
	default:
	{
		if (sensor.AAT_flash_control != NULL)
			sensor.AAT_flash_control(sd, FLASH_CONTROL_OFF);
		else
			CAM_ERROR_MSG(&client->dev, "%s %s :AAT_flash_control is NULL!!!s \n", sensor.name, __func__);

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
static int camdrv_ss_set_frame_rate(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s %s :camdrv_ss_set_frame_rate  E  = %d \n", sensor.name, __func__, ctrl->value);

	switch (ctrl->value) {
	case FRAME_RATE_AUTO:
	{
		if (sensor.fps_auto_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : FRAME_RATE_AUTO not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err =  camdrv_ss_i2c_set_config_register(client, sensor.fps_auto_regs, sensor.rows_num_fps_auto_regs, "fps_auto_regs");
		break;
	}

	case FRAME_RATE_5:
	{
		if (sensor.fps_5_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : fps_5_regs not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err =  camdrv_ss_i2c_set_config_register(client, sensor.fps_5_regs, sensor.rows_num_fps_5_regs, "fps_5_regs");
		break;
	}

	case FRAME_RATE_7:
	{
		if (sensor.fps_7_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : FRAME_RATE_7 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_7_regs, sensor.rows_num_fps_7_regs, "fps_7_regs");
		break;
	}

	case FRAME_RATE_10:
	{
		if (sensor.fps_10_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : FRAME_RATE_10 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_10_regs, sensor.rows_num_fps_10_regs, "fps_10_regs");
		break;
	}

	case FRAME_RATE_15:
	{
		if (sensor.fps_15_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : FRAME_RATE_15 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_15_regs, sensor.rows_num_fps_15_regs, "fps_15_regs");
		break;
	}

	case FRAME_RATE_20:
	{
		if (sensor.fps_20_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : fps_20_regs not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_20_regs, sensor.rows_num_fps_20_regs, "fps_20_regs");
		break;
	}

	case FRAME_RATE_25:
	{
		if (sensor.fps_25_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : FRAME_RATE_25 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_25_regs, sensor.rows_num_fps_25_regs, "fps_25_regs");
		break;
	}

	case FRAME_RATE_30:
	{
		if (sensor.fps_30_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : FRAME_RATE_30 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_30_regs, sensor.rows_num_fps_30_regs, "fps_30_regs");
		break;
	}

	case FRAME_RATE_60:
	{
		if (sensor.fps_60_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : fps_60_regs not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_60_regs, sensor.rows_num_fps_60_regs, "fps_60_regs");
		break;
	}

	case FRAME_RATE_120:
	{
		if (sensor.fps_120_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : FRAME_RATE_120 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.fps_120_regs, sensor.rows_num_fps_120_regs, "fps_120_regs");
		break;
	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : FRAME_RATE_DEFAULT not supported !!! \n", sensor.name, __func__);
		err = -EIO;

		break;
	}
	}

	if (ctrl->value == FRAME_RATE_AUTO) {
		state->fps = 30;    /* default 30 */
	} else {
		state->fps = ctrl->value;
	}

	return err;
}

/**************************************************************************
 * camdrv_ss_set_preview_stop
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/

static int camdrv_ss_set_dataline_onoff(struct v4l2_subdev *sd, int onoff)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	if (onoff) {
		/* data line on */
		CAM_INFO_MSG(&client->dev, "%s %s :ON !! \n", sensor.name, __func__);

		if (sensor.pattern_on_regs == NULL)
			CAM_ERROR_MSG(&client->dev, "%s %s : pattern_on_regs is NULL, please check if it is needed !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.pattern_on_regs, sensor.rows_num_pattern_on_regs, "pattern_on_regs");

		if (err < 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : i2c failed !! \n", sensor.name, __func__);
			return -EIO;
		}
	} else {
		/* data line off */
		CAM_INFO_MSG(&client->dev, "%s %s :OFF !! \n", sensor.name, __func__);

		if (sensor.pattern_off_regs == NULL)
			CAM_ERROR_MSG(&client->dev, "%s %s : pattern_off_regs is NULL, please check if it is needed !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.pattern_off_regs, sensor.rows_num_pattern_off_regs, "pattern_off_regs");

		if (err < 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : i2c failed !! \n", sensor.name, __func__);
			return -EIO;
		}

		state->check_dataline = CHK_DATALINE_OFF;
	}
	msleep(100);

	CAM_INFO_MSG(&client->dev, "%s %s :done . \n", sensor.name, __func__);

	return err;
}

static int  camdrv_ss_set_preview_stop(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);

	if (state->runmode == CAMDRV_SS_RUNNING_MODE_RUNNING) {
		state->runmode = CAMDRV_SS_RUNNING_MODE_IDLE;
	}

	CAM_INFO_MSG(&client->dev, "%s %s : camdrv_ss_set_preview_stop  \n", sensor.name, __func__);
	return 0;
}


/**************************************************************************
 * camdrv_ss_set_dzoom
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int camdrv_ss_set_dzoom(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s %s : camdrv_ss_set_dzoom = %d  \n", sensor.name, __func__, ctrl->value);

	switch (ctrl->value) {
	case ZOOM_LEVEL_0:
	{
		if (sensor.zoom_00_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : ZOOM_LEVEL_0 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_00_regs, sensor.rows_num_zoom_00_regs, "zoom_00_regs");
		break;
	}

	case ZOOM_LEVEL_1:
	{
		if (sensor.zoom_01_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : ZOOM_LEVEL_1 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_01_regs, sensor.rows_num_zoom_01_regs, "zoom_01_regs");
		break;
	}

	case ZOOM_LEVEL_2:
	{
		if (sensor.zoom_02_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : ZOOM_LEVEL_2 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_02_regs, sensor.rows_num_zoom_02_regs, "zoom_02_regs");
		break;
	}

	case ZOOM_LEVEL_3:
	{
		if (sensor.zoom_03_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : ZOOM_LEVEL_3 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_03_regs, sensor.rows_num_zoom_03_regs, "zoom_03_regs");
		break;
	}

	case ZOOM_LEVEL_4:
	{
		if (sensor.zoom_04_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : ZOOM_LEVEL_4 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_04_regs, sensor.rows_num_zoom_04_regs, "zoom_04_regs");
		break;
	}

	case ZOOM_LEVEL_5:
	{
		if (sensor.zoom_05_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : ZOOM_LEVEL_5 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_05_regs, sensor.rows_num_zoom_05_regs, "zoom_05_regs");
		break;
	}

	case ZOOM_LEVEL_6:
	{
		if (sensor.zoom_06_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : ZOOM_LEVEL_6 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_06_regs, sensor.rows_num_zoom_06_regs, "zoom_06_regs");
		break;
	}

	case ZOOM_LEVEL_7:
	{
		if (sensor.zoom_07_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : ZOOM_LEVEL_7 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_07_regs, sensor.rows_num_zoom_07_regs, "zoom_07_regs");
		break;
	}

	case ZOOM_LEVEL_8:
	{
		if (sensor.zoom_08_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : ZOOM_LEVEL_8 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.zoom_08_regs, sensor.rows_num_zoom_08_regs, "zoom_08_regs");
		break;
	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : default not supported !!! \n", sensor.name, __func__);
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
static int camdrv_ss_set_preview_size(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0, index;

	index = state->preview_framesize_index;

	CAM_INFO_MSG(&client->dev, "%s %s : index = %d  \n", sensor.name, __func__, index);

	switch (index) {
	case PREVIEW_SIZE_QCIF:
	{
		if (sensor.preview_size_176x144_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_QCIF not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_176x144_regs, sensor.rows_num_preview_size_176x144_regs, "preview_size_176x144_regs");
		break;
	}

	case PREVIEW_SIZE_QVGA:
	{
		if (sensor.preview_size_320x240_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_QVGA not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_320x240_regs, sensor.rows_num_preview_size_320x240_regs, "preview_size_320x240_regs");
		break;
	}

	case PREVIEW_SIZE_CIF:
	{
		if (sensor.preview_size_352x288_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_CIF not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_352x288_regs, sensor.rows_num_preview_size_352x288_regs, "preview_size_352x288_regs");
		break;
	}

	case PREVIEW_SIZE_VGA:
	{
		if (sensor.preview_size_640x480_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_VGA not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_640x480_regs, sensor.rows_num_preview_size_640x480_regs, "preview_size_640x480_regs");
		break;
	}

	case PREVIEW_SIZE_4CIF:
	{
		if (sensor.preview_size_704x576_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_4CIF not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_704x576_regs, sensor.rows_num_preview_size_704x576_regs, "preview_size_704x576_regs");
		break;
	}

	case PREVIEW_SIZE_D1:
	{
		if (sensor.preview_size_720x480_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_D1 not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_720x480_regs, sensor.rows_num_preview_size_720x480_regs, "preview_size_720x480_regs");
		break;
	}

	case PREVIEW_SIZE_WVGA:
	{
		if (sensor.preview_size_800x480_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_WVGA not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_800x480_regs, sensor.rows_num_preview_size_800x480_regs, "preview_size_800x480_regs");
		break;
	}

	case PREVIEW_SIZE_SVGA:
	{
		if (sensor.preview_size_800x600_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_SVGA not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_800x600_regs, sensor.rows_num_preview_size_800x600_regs, "preview_size_800x600_regs");
		break;
	}

	case PREVIEW_SIZE_WSVGA:
	{
		if (sensor.preview_size_1024x600_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_WSVGA not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_1024x600_regs, sensor.rows_num_preview_size_1024x600_regs, "preview_size_1024x600_regs");
		break;
	}

	case PREVIEW_SIZE_XGA:
	{
		if (sensor.preview_size_1024x768_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_WSVGA not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_1024x768_regs, sensor.rows_num_preview_size_1024x768_regs, "preview_size_1024x768_regs");
		break;
	}

	case PREVIEW_SIZE_1MP:
	{
		if (sensor.preview_size_1280x960_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_1MP not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_1280x960_regs, sensor.rows_num_preview_size_1280x960_regs, "preview_size_1280x960_regs");
		break;
	}

	case PREVIEW_SIZE_W1MP:
	{
		if (sensor.preview_size_1600x960_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_W1MP not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_1600x960_regs, sensor.rows_num_preview_size_1600x960_regs, "preview_size_1600x960_regs");
		break;
	}

	case PREVIEW_SIZE_2MP:
	{
		if (sensor.preview_size_1600x1200_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_W1MP not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_1600x1200_regs, sensor.rows_num_preview_size_1600x1200_regs, "preview_size_1600x1200_regs");
		break;
	}

	case PREVIEW_SIZE_W2MP:
	{
		if (sensor.preview_size_2048x1232_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_W1MP not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_2048x1232_regs, sensor.rows_num_preview_size_2048x1232_regs, "preview_size_2048x1232_regs");
		break;
	}

	case PREVIEW_SIZE_3MP:
	{
		if (sensor.preview_size_2048x1536_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_W1MP not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_2048x1536_regs, sensor.rows_num_preview_size_2048x1536_regs, "preview_size_2048x1232_regs");
		break;
	}

	case PREVIEW_SIZE_5MP:
	{
		if (sensor.preview_size_2560x1920_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : PREVIEW_SIZE_W1MP not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_size_2560x1920_regs, sensor.rows_num_preview_size_2560x1920_regs, "preview_size_2560x1920_regs");
		break;
	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : default case not supported !!! \n", sensor.name, __func__);
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

	CAM_INFO_MSG(&client->dev, "%s %s :  \n", sensor.name, __func__);

	if (!state->pix.width || !state->pix.height) {
		CAM_ERROR_MSG(&client->dev, "%s %s : width or height is NULL!!! \n", sensor.name, __func__);
		return -EINVAL;
	}

	if (state->runmode == CAMDRV_SS_RUNNING_MODE_CAPTURE) {
		if (sensor.preview_camera_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : Returned after a capture, preview_camera_regs is NULL  !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.preview_camera_regs, sensor.rows_num_preview_camera_regs, "preview_camera_regs");

		if (err < 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : I2C preview_camera_regs IS FAILED  \n", sensor.name, __func__);
			return -EIO;
		}
	}

	err = camdrv_ss_set_preview_size(sd);
	if (err < 0) {
		CAM_ERROR_MSG(&client->dev, "%s %s : camdrv_ss_set_preview_size is FAILED !! \n", sensor.name, __func__);
		return -EIO;
	}

	state->runmode = CAMDRV_SS_RUNNING_MODE_RUNNING;
	state->camera_flash_fire = 0;
	state->camera_af_flash_checked = 0;

	if (state->check_dataline) { /* Output Test Pattern */
		err = camdrv_ss_set_dataline_onoff(sd, 1);
		if (err < 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : check_dataline is FAILED !! \n", sensor.name, __func__);
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

	CAM_INFO_MSG(&client->dev, "%s %s :  quality =%d \n", sensor.name, __func__, quality);
	x = 100/JPEG_QUALITY_MAX;
	quality = quality / x;
	if (quality >= JPEG_QUALITY_MAX)
		quality = JPEG_QUALITY_SUPERFINE;

	switch (quality) {
	case JPEG_QUALITY_SUPERFINE:
	{
		if (sensor.quality_superfine_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : JPEG_QUALITY_SUPERFINE not supported ,trying  below quality!!! \n", sensor.name, __func__);
		} else {
			err = camdrv_ss_i2c_set_config_register(client, sensor.quality_superfine_regs, sensor.rows_num_quality_superfine_regs, "quality_superfine_regs");
			break;
		}
	}

	case JPEG_QUALITY_FINE:
	{
		if (sensor.quality_fine_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : JPEG_QUALITY_FINE not supported , trying  below quality !!! \n", sensor.name, __func__);
		} else {
			err = camdrv_ss_i2c_set_config_register(client, sensor.quality_fine_regs, sensor.rows_num_quality_fine_regs, "quality_fine_regs");
			break;
		}
	}

	case JPEG_QUALITY_NORMAL:
	{
		if (sensor.quality_normal_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : JPEG_QUALITY_NORMAL not supported , trying  below quality!! \n", sensor.name, __func__);
		} else {
			err = camdrv_ss_i2c_set_config_register(client, sensor.quality_normal_regs, sensor.rows_num_quality_normal_regs, "quality_normal_regs");
			break;
		}
	}

	case JPEG_QUALITY_ECONOMY:
	{
		if (sensor.quality_economy_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : JPEG_QUALITY_ECONOMY not supported , trying  below quality!!! \n", sensor.name, __func__);
		} else {
			err = camdrv_ss_i2c_set_config_register(client, sensor.quality_economy_regs, sensor.rows_num_quality_economy_regs, "quality_economy_regs");
			break;
		}
	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : no level is  supported !!! \n", sensor.name, __func__);
		err = -EINVAL;
		break;
	}
	}

	return err;
}


static int camdrv_ss_set_capture_size(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0, index;
	int rows_num_ = 0;

	index = state->capture_framesize_index;
	printk("%s \n", __func__);

	CAM_INFO_MSG(&client->dev, "%s %s :   index =%d \n", sensor.name, __func__, index);

	switch (index) {
	/* ======================= 1.333 Ratio ======================================= */
	case CAPTURE_SIZE_VGA: /* 416x320 */
	{
		if (sensor.capture_size_640x480_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : capture_size_640x480_regs not supported !!! \n", sensor.name, __func__);
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
			CAM_ERROR_MSG(&client->dev, "%s %s : capture_size_800x480_regs not supported !!! \n", sensor.name, __func__);
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
			CAM_ERROR_MSG(&client->dev, "%s %s : capture_size_800x600_regs not supported !!! \n", sensor.name, __func__);
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
			CAM_ERROR_MSG(&client->dev, "%s %s : capture_size_720x480_regs not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_720x480_regs, sensor.rows_num_capture_size_720x480_regs, "capture_size_720x480_regs");

		state->postview_info.width = 720;
		state->postview_info.height = 480;
		break;
	}

	case CAPTURE_SIZE_WSVGA: 	/* 1024x600 */
	{
		if (sensor.capture_size_1024x600_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : capture_size_1024x600_regs not supported !!! \n", sensor.name, __func__);
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
			CAM_ERROR_MSG(&client->dev, "%s %s : capture_size_1024x768_regs not supported !!! \n", sensor.name, __func__);
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
			CAM_ERROR_MSG(&client->dev, "%s %s : capture_size_1280x960_regs not supported !!! \n", sensor.name, __func__);
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
			CAM_ERROR_MSG(&client->dev, "%s %s : capture_size_1600x960_regs not supported !!! \n", sensor.name, __func__);
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
			CAM_ERROR_MSG(&client->dev, "%s %s : capture_size_1600x1200_regs not supported !!! \n", sensor.name, __func__);
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
			CAM_ERROR_MSG(&client->dev, "%s %s : capture_size_2048x1232_regs not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_2048x1232_regs, sensor.rows_num_capture_size_2048x1232_regs, "capture_size_2048x1232_regs");

		state->postview_info.width = 2048;
		state->postview_info.height = 1232;
		break;
	}

	case CAPTURE_SIZE_3MP:	/*2048x1536*/
	{
		if (sensor.capture_size_2048x1536_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : capture_size_2048x1536_regs not supported !!! \n", sensor.name, __func__);
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
			CAM_ERROR_MSG(&client->dev, "%s %s : capture_size_2560x1920_regs not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.capture_size_2560x1920_regs, sensor.rows_num_capture_size_2560x1920_regs, "capture_size_2560x1920_regs");

		state->postview_info.width = 2560;
		state->postview_info.height = 1920;
		break;
	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : default case not supported !!! \n", sensor.name, __func__);
		err = -EINVAL;
		break;
	}
	}

	return err;
}

static int camdrv_ss_set_capture_start(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;
	int light_state = CAM_NORMAL_LIGHT;

	bCaptureMode = true;
	/* Initialize... */
	state->isoSpeedRating = 100;
	state->exposureTime = 0;

	printk("%s \n", __func__);
	/* Set image size */
	err = camdrv_ss_set_capture_size(sd);
	if (err < 0) {
		CAM_ERROR_MSG(&client->dev, "%s %s : camdrv_ss_set_capture_size not supported !!! \n", sensor.name, __func__);
		return -EIO;
	}
	state->runmode = CAMDRV_SS_RUNNING_MODE_CAPTURE;

	if (state->currentScene == SCENE_MODE_NIGHTSHOT || state->currentScene == SCENE_MODE_FIREWORKS) {
		/* Set Snapshot registers */
		if (sensor.snapshot_nightmode_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : snapshot_nightmode_regs not supported !!! \n", sensor.name, __func__);
			err = -EIO;
		}

		err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_nightmode_regs, sensor.rows_num_snapshot_nightmode_regs, "snapshot_nightmode_regs");
		if (err < 0) {
			CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Could not take a picture\n", __FILE__, __LINE__);
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
					CAM_ERROR_MSG(&client->dev, "%s %s : check_flash_needed is NULL !!! \n", sensor.name, __func__);

				if (bflash_needed) {
					state->camera_flash_fire = 1;
				}
			}
		}

		if (state->camera_flash_fire) {
			/* Set Snapshot registers */
			if (sensor.snapshot_flash_on_regs == 0) {
				CAM_ERROR_MSG(&client->dev, "%s %s : snapshot_lowlight_regs not supported !!! \n", sensor.name, __func__);
				err = -EIO;
			}
			err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_flash_on_regs, sensor.rows_num_snapshot_flash_on_regs,
							"snapshot_flash_on_regs");
			if (err < 0) {
				CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Couldn't Set Flash_on_regs \n", __FILE__, __LINE__);
			}

				sensor.AAT_flash_control(sd, FLASH_CONTROL_MAX_LEVEL);
		}

		if (sensor.get_light_condition != NULL) {
			sensor.get_light_condition(sd, &light_state);
			CAM_ERROR_MSG(&client->dev, "%s %s : light_state =%d !!\n", sensor.name, __func__, light_state);
		}

		if (light_state == CAM_LOW_LIGHT) {
			if (sensor.snapshot_lowlight_regs == 0) {
				CAM_ERROR_MSG(&client->dev, "%s %s : snapshot_lowlight_regs not supported !!! \n", sensor.name, __func__);
				err = -EIO;
			}

			err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_lowlight_regs, sensor.rows_num_snapshot_lowlight_regs, "snapshot_highlight_regs");
			if (err < 0) {
				CAM_ERROR_MSG(&client->dev, "%s %s : camdrv_ss_i2c_set_config_register failed  not supported !!! \n", sensor.name, __func__);
				return -EIO;
			}
		} else if (light_state == CAM_HIGH_LIGHT) {
			/* Set Snapshot registers */
			if (sensor.snapshot_highlight_regs == 0) {
				CAM_ERROR_MSG(&client->dev, "%s %s : snapshot_highlight_regs not supported !!! \n", sensor.name, __func__);
				err = -EIO;
			}

			err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_highlight_regs, sensor.rows_num_snapshot_highlight_regs, "snapshot_highlight_regs");
			if (err < 0) {
				CAM_ERROR_MSG(&client->dev, "%s %s : camdrv_ss_i2c_set_config_register failed  not supported !!! \n", sensor.name, __func__);
				return -EIO;
			}
		} else { /* CAM_NORMAL LIGHT */
			/* Set Snapshot registers */
			if (sensor.snapshot_normal_regs == 0) {
				CAM_ERROR_MSG(&client->dev, "%s %s : snapshot_normal_regs not supported !!! \n", sensor.name, __func__);
				err = -EIO;
			}

			err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_normal_regs, sensor.rows_num_snapshot_normal_regs, "snapshot_normal_regs");
			if (err < 0) {
				CAM_ERROR_MSG(&client->dev, "%s %s : camdrv_ss_i2c_set_config_register failed  not supported !!! \n", sensor.name, __func__);
				return -EIO;
			}
		}

	}

	/* Get iso speed rating and exposure time for EXIF. */
	state->isoSpeedRating = sensor.get_iso_speed_rate(sd);
	state->exposureTime = sensor.get_shutterspeed(sd);

	return 0;
}

static int camdrv_ss_set_capture_done(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s\n", __func__);

	if (state->camera_af_flash_fire) {
		if (sensor.snapshot_af_preflash_off_regs == 0) {
			CAM_ERROR_MSG(&client->dev, "%s %s : snapshot_af_preflash_off_regs not supported !!! \n", sensor.name, __func__);
		} else
			err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_af_preflash_off_regs, sensor.rows_num_snapshot_af_preflash_off_regs, "snapshot_af_preflash_off_regs");

		if (err < 0) {
			CAM_ERROR_MSG(&client->dev, "[%s: %d] ERROR! Setting af_preflash_off_regs\n", __FILE__, __LINE__);
		}
	}

	if (sensor.AAT_flash_control != NULL)
		sensor.AAT_flash_control(sd, FLASH_CONTROL_OFF);

	bCaptureMode = false;

	return 0;
}


static int camdrv_ss_set_scene_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s %s :   value =%d \n", sensor.name, __func__, ctrl->value);

	if (ctrl->value != SCENE_MODE_NONE) {
		if (sensor.scene_none_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : scene_none_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_none_regs, sensor.rows_num_scene_none_regs, "scene_none_regs");
		if (err < 0) {
			CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Could not take a picture\n", __FILE__, __LINE__);
			return -EIO;
		}
	}

	switch (ctrl->value) {
	case SCENE_MODE_NONE:
	{
		if (sensor.scene_none_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : scene_none_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_none_regs, sensor.rows_num_scene_none_regs, "scene_none_regs");
		break;
	}

	case SCENE_MODE_PORTRAIT:
	{
		/* Metering-Center, EV0, WB-Auto, Sharp-1, Sat0, AF-Auto will be set in HAL layer */

		if (sensor.scene_portrait_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : scene_portrait_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_portrait_regs, sensor.rows_num_scene_portrait_regs, "scene_portrait_regs");
		break;
	}

	case SCENE_MODE_NIGHTSHOT:
	{
		if (sensor.scene_nightshot_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : scene_nightshot_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_nightshot_regs, sensor.rows_num_scene_nightshot_regs, "scene_nightshot_regs");
		break;
	}

	case SCENE_MODE_BACK_LIGHT:
	{
		/* Metering-Spot, EV0, WB-Auto, Sharp0, Sat0, AF-Auto will be set in HAL layer */
		if (sensor.scene_backlight_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : scene_backlight_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_backlight_regs, sensor.rows_num_scene_backlight_regs, "scene_backlight_regs");
		break;
	}

	case SCENE_MODE_LANDSCAPE:
	{
		/* Metering-Matrix, EV0, WB-Auto, Sharp+1, Sat+1, AF-Auto will be set in HAL layer */

		if (sensor.scene_landscape_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : scene_landscape_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_landscape_regs, sensor.rows_num_scene_landscape_regs, "scene_landscape_regs");

		break;
	}

	case SCENE_MODE_SPORTS:
	{
		if (sensor.scene_sports_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : scene_sports_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_sports_regs, sensor.rows_num_scene_sports_regs, "scene_sports_regs");

		break;
	}

	case SCENE_MODE_PARTY_INDOOR:
	{
		if (sensor.scene_party_indoor_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : scene_party_indoor_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_party_indoor_regs, sensor.rows_num_scene_party_indoor_regs, "scene_party_indoor_regs");

		break;
	}

	case SCENE_MODE_BEACH_SNOW:
	{
		/* Metering-Center, EV+1, WB-Auto, Sharp0, Sat+1, AF-Auto will be set in HAL layer */
		if (sensor.scene_beach_snow_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : saturation_plus_1_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_beach_snow_regs, sensor.rows_num_scene_beach_snow_regs, "scene_beach_snow_regs");

		break;
	}

	case SCENE_MODE_SUNSET:
	{
		/* Metering-Center, EV0, WB-daylight, Sharp0, Sat0, AF-Auto will be set in HAL layer */
		if (sensor.scene_sunset_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : scene_sunset_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_sunset_regs, sensor.rows_num_scene_sunset_regs, "scene_sunset_regs");
		break;
	}

	case SCENE_MODE_DUSK_DAWN:
	{
		/* Metering-Center, EV0, WB-fluorescent, Sharp0, Sat0, AF-Auto will be set in HAL layer */
		if (sensor.scene_duskdawn_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : scene_duskdawn_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_duskdawn_regs, sensor.rows_num_scene_duskdawn_regs, "scene_duskdawn_regs");

		break;
	}

	case SCENE_MODE_FALL_COLOR:
	{
		/* Metering-Center, EV0, WB-Auto, Sharp0, Sat+2, AF-Auto will be set in HAL layer */
		if (sensor.scene_fall_color_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : scene_fall_color_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_fall_color_regs, sensor.rows_num_scene_fall_color_regs, "scene_fall_color_regs");

		break;
	}

	case SCENE_MODE_FIREWORKS:
	{
		if (sensor.scene_fireworks_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : scene_fireworks_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_fireworks_regs, sensor.rows_num_scene_fireworks_regs, "scene_fireworks_regs");

		break;
	}

	case SCENE_MODE_TEXT:
	{
		/* Metering-Center, EV0, WB-Auto, Sharp+2, Sat0, AF-Macro will be set in HAL layer */
		if (sensor.scene_text_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : scene_text_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_text_regs, sensor.rows_num_scene_text_regs, "scene_text_regs");

		break;
	}

	case SCENE_MODE_CANDLE_LIGHT:
	{
		/* Metering-Center, EV0, WB-Daylight, Sharp0, Sat0, AF-Auto will be set in HAL layer */
		if (sensor.scene_candle_light_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : scene_candle_light_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.scene_candle_light_regs, sensor.rows_num_scene_candle_light_regs, "scene_candle_light_regs");

		break;
	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : default not supported !!! \n", sensor.name, __func__);
		err = -EINVAL;
		break;
	}
	}

	state->currentScene = ctrl->value;

	return err;
}


static int camdrv_ss_set_effect(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s %s :  value =%d \n", sensor.name, __func__, ctrl->value);

	switch (ctrl->value) {
	case IMAGE_EFFECT_NONE:
	{
		if (sensor.effect_normal_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : effect_normal_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_normal_regs, sensor.rows_num_effect_normal_regs, "effect_normal_regs");

		break;
	}

	case IMAGE_EFFECT_MONO:
	{
		if (sensor.effect_mono_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : effect_mono_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_mono_regs, sensor.rows_num_effect_mono_regs, "effect_mono_regs");

	break;
	}

	case IMAGE_EFFECT_SEPIA:
	case IMAGE_EFFECT_ANTIQUE:
	{
		if (sensor.effect_sepia_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : effect_sepia_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_sepia_regs, sensor.rows_num_effect_sepia_regs, "effect_sepia_regs");

		break;
	}

	case IMAGE_EFFECT_NEGATIVE:
	{
		if (sensor.effect_negative_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : effect_negative_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_negative_regs, sensor.rows_num_effect_negative_regs, "effect_negative_regs");

		break;
	}

	case IMAGE_EFFECT_AQUA:
	{
		if (sensor.effect_aqua_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : effect_aqua_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_aqua_regs, sensor.rows_num_effect_aqua_regs, "effect_aqua_regs");

		break;
	}

	case IMAGE_EFFECT_BNW:
	{
		if (sensor.effect_black_white_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : effect_black_white_regsnot supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_black_white_regs, sensor.rows_num_effect_black_white_regs, "effect_black_white_regs");

		break;
	}

	case IMAGE_EFFECT_SOLARIZATION:
	{
		if (sensor.effect_solarization_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : effect_solarization_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_solarization_regs, sensor.rows_num_effect_solarization_regs, "effect_solarization_regs");

		break;
	}

	case IMAGE_EFFECT_SHARPEN:
	{
		if (sensor.effect_sharpen_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : effect_sharpen_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.effect_sharpen_regs, sensor.rows_num_effect_sharpen_regs, "effect_sharpen_regs");

		break;
	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : default case not supported !!! \n", sensor.name, __func__);
		break;
	}
	}

	return err;
}


static int camdrv_ss_set_white_balance(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s %s :  value =%d \n", sensor.name, __func__, ctrl->value);

	switch (ctrl->value) {
	case WHITE_BALANCE_AUTO:
	{
		if (sensor.wb_auto_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : wb_auto_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_auto_regs, sensor.rows_num_wb_auto_regs, "wb_auto_regs");

		break;
	}

	case WHITE_BALANCE_SUNNY:
	{
		if (sensor.wb_sunny_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : wb_sunny_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_sunny_regs, sensor.rows_num_wb_sunny_regs, "wb_sunny_regs");

		break;
	}

	case WHITE_BALANCE_CLOUDY:
	{
		if (sensor.wb_cloudy_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : wb_cloudy_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_cloudy_regs, sensor.rows_num_wb_cloudy_regs, "wb_cloudy_regs");

		break;
	}

	case WHITE_BALANCE_TUNGSTEN:  /* WHITE_BALANCE_INCANDESCENT: */
	{
		if (sensor.wb_tungsten_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : wb_tungsten_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_tungsten_regs, sensor.rows_num_wb_tungsten_regs, "wb_tungsten_regs");

		break;
	}

	case WHITE_BALANCE_FLUORESCENT:
	{
		if (sensor.wb_fluorescent_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : wb_fluorescent_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_fluorescent_regs, sensor.rows_num_wb_fluorescent_regs, "wb_fluorescent_regs");

		break;
	}

	case WHITE_BALANCE_CWF:
	{
		if (sensor.wb_cwf_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : wb_cwf_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.wb_cwf_regs, sensor.rows_num_wb_cwf_regs, "wb_cwf_regs");

		break;
	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : default not supported !!! \n", sensor.name, __func__);
		break;
	}
	}

	state->currentWB = ctrl->value;

	return err;
}


static int camdrv_ss_set_metering(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s %s :  value =%d \n", sensor.name, __func__, ctrl->value);

	switch (ctrl->value) {
	case METERING_MATRIX:
	{
		if (sensor.metering_matrix_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : metering_matrix_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.metering_matrix_regs, sensor.rows_num_metering_matrix_regs, "metering_matrix_regs");

		break;
	}

	case METERING_CENTER:
	{
		if (sensor.metering_center_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : metering_center_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.metering_center_regs, sensor.rows_num_metering_center_regs, "metering_center_regs");

		break;
	}

	case METERING_SPOT:
	{

		if (sensor.metering_spot_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : metering_spot_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.metering_spot_regs, sensor.rows_num_metering_spot_regs, "metering_spot_regs");

		break;

	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : default not supported !!! \n", sensor.name, __func__);
		break;
	}
	}

	state->currentMetering = ctrl->value;

	return err;
}


static int camdrv_ss_set_iso(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	CAM_INFO_MSG(&client->dev, "%s %s :  NEED TO ADD ISO SUPPORT !!d \n", sensor.name, __func__);
	return 0;
}


static int camdrv_ss_set_ev(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;
	struct camdrv_ss_state *state = to_state(sd);

	CAM_INFO_MSG(&client->dev, "%s %s :  value =%d \n", sensor.name, __func__, ctrl->value);

	switch (ctrl->value) {
	case EV_MINUS_4:
	{
		if (sensor.ev_minus_4_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : ev_minus_4_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_minus_4_regs, sensor.rows_num_ev_minus_4_regs, "ev_minus_4_regs");

		break;
	}

	case EV_MINUS_3:
	{
		if (sensor.ev_minus_3_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : ev_minus_3_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_minus_3_regs, sensor.rows_num_ev_minus_3_regs, "ev_minus_3_regs");

		break;
	}

	case EV_MINUS_2:
	{
		if (sensor.ev_minus_2_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : ev_minus_2_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_minus_2_regs, sensor.rows_num_ev_minus_2_regs, "ev_minus_2_regs");

		break;
	}

	case EV_MINUS_1:
	{
		if (sensor.ev_minus_1_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : ev_minus_1_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_minus_1_regs, sensor.rows_num_ev_minus_1_regs, "ev_minus_1_regs");

		break;
	}

	case EV_DEFAULT:
	{
		if (sensor.ev_default_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : ev_default_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_default_regs, sensor.rows_num_ev_default_regs, "ev_default_regs");

		break;
	}

	case EV_PLUS_1:
	{
		if (sensor.ev_plus_1_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : ev_plus_1_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_plus_1_regs, sensor.rows_num_ev_plus_1_regs, "ev_plus_1_regs");

		break;
	}

	case EV_PLUS_2:
	{
		if (sensor.ev_plus_2_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : ev_plus_2_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_plus_2_regs, sensor.rows_num_ev_plus_2_regs, "ev_plus_2_regs");

		break;
	}

	case EV_PLUS_3:
	{
		if (sensor.ev_plus_3_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : ev_plus_3_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_plus_3_regs, sensor.rows_num_ev_plus_3_regs, "ev_plus_3_regs");

		break;
	}

	case EV_PLUS_4:
	{
		if (sensor.ev_plus_4_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : ev_plus_4_regs not supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ev_plus_4_regs, sensor.rows_num_ev_plus_4_regs, "ev_plus_4_regs");

		break;
	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : default case supported !!! \n", sensor.name, __func__);
		break;
	}
	}

	return err;
}


static int camdrv_ss_set_saturation(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s %s :  value =%d \n", sensor.name, __func__, ctrl->value);

	switch (ctrl->value) {
	case SATURATION_MINUS_2:
	{
		if (sensor.saturation_minus_2_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : saturation_minus_2_regs supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.saturation_minus_2_regs, sensor.rows_num_saturation_minus_2_regs, "saturation_minus_2_regs");

		break;
	}

	case SATURATION_MINUS_1:
	{
		if (sensor.saturation_minus_1_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : saturation_minus_1_regs supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.saturation_minus_1_regs, sensor.rows_num_saturation_minus_1_regs, "saturation_minus_1_regs");

		break;
	}

	case SATURATION_DEFAULT:
	{
		if (sensor.saturation_default_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : saturation_default_regs supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.saturation_default_regs, sensor.rows_num_saturation_default_regs, "saturation_default_regs");

		break;
	}

	case SATURATION_PLUS_1:
	{
		if (sensor.saturation_plus_1_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s :  saturation_plus_1_regs supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.saturation_plus_1_regs, sensor.rows_num_saturation_plus_1_regs, "saturation_plus_1_regs");

		break;
	}

	case SATURATION_PLUS_2:
	{
		if (sensor.saturation_plus_2_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : saturation_plus_2_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.saturation_plus_2_regs, sensor.rows_num_saturation_plus_2_regs, "saturation_plus_2_regs");

		break;
	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : default case  supported !!! \n", sensor.name, __func__);
		break;
	}
	}

	return err;
}


static int camdrv_ss_set_contrast(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s %s :  value =%d \n", sensor.name, __func__, ctrl->value);

	switch (ctrl->value) {
	case CONTRAST_MINUS_2:
	{
		if (sensor.contrast_minus_2_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : contrast_minus_2_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.contrast_minus_2_regs, sensor.rows_num_contrast_minus_2_regs, "contrast_minus_2_regs");

		break;
	}

	case CONTRAST_MINUS_1:
	{
		if (sensor.contrast_minus_1_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : contrast_minus_1_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.contrast_minus_1_regs, sensor.rows_num_contrast_minus_1_regs, "contrast_minus_1_regs");

		break;
	}

	case CONTRAST_DEFAULT:
	{
		if (sensor.contrast_default_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : contrast_default_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.contrast_default_regs, sensor.rows_num_contrast_default_regs, "contrast_default_regs");

		break;
	}

	case CONTRAST_PLUS_1:
	{
		if (sensor.contrast_plus_1_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : contrast_plus_1_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.contrast_plus_1_regs, sensor.rows_num_contrast_plus_1_regs, "contrast_plus_1_regs");

		break;
	}

	case CONTRAST_PLUS_2:
	{
		if (sensor.contrast_plus_2_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : contrast_plus_2_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.contrast_plus_2_regs, sensor.rows_num_contrast_plus_2_regs, "contrast_plus_2_regs");

		break;

	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : default case  supported !!! \n", sensor.name, __func__);
		break;
	}
	}


	return err;
}

static int camdrv_ss_set_sharpness(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s %s :  value =%d \n", sensor.name, __func__, ctrl->value);

	switch (ctrl->value) {
	case SHARPNESS_MINUS_2:
	{
		if (sensor.sharpness_minus_2_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : sharpness_minus_2_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.sharpness_minus_2_regs, sensor.rows_num_sharpness_minus_2_regs, "sharpness_minus_2_regs");

		break;
	}

	case SHARPNESS_MINUS_1:
	{
		if (sensor.sharpness_minus_1_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : sharpness_minus_1_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.sharpness_minus_1_regs, sensor.rows_num_sharpness_minus_1_regs, "sharpness_minus_1_regs");

		break;
	}

	case SHARPNESS_DEFAULT:
	{
		if (sensor.sharpness_default_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : sharpness_default_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.sharpness_default_regs, sensor.rows_num_sharpness_default_regs, "sharpness_default_regs");

		break;
	}

	case SHARPNESS_PLUS_1:
	{
		if (sensor.sharpness_plus_1_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : sharpness_plus_1_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.sharpness_plus_1_regs, sensor.rows_num_sharpness_plus_1_regs, "sharpness_plus_1_regs");

		break;
	}

	case SHARPNESS_PLUS_2:
	{
		if (sensor.sharpness_plus_2_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : sharpness_plus_2_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.sharpness_plus_2_regs, sensor.rows_num_sharpness_plus_2_regs, "sharpness_plus_2_regs");

		break;
	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : default case  supported !!! \n", sensor.name, __func__);
		break;
	}
	} /* end of switch */

	return err;
}


static int camdrv_ss_set_focus_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s %s :  value =%d \n", sensor.name, __func__, ctrl->value);

	switch (ctrl->value) {
	case FOCUS_MODE_MACRO:
	{
		if (sensor.af_macro_mode_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : af_macro_mode_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.af_macro_mode_regs, sensor.rows_num_af_macro_mode_regs, "af_macro_mode_regs");

		break;
	}

	case FOCUS_MODE_AUTO:
	{
		if (sensor.af_normal_mode_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : af_normal_mode_regs  supported !!! \n", sensor.name, __func__);
		else
			err |= camdrv_ss_i2c_set_config_register(client, sensor.af_normal_mode_regs, sensor.rows_num_af_normal_mode_regs, "af_normal_mode_regs");

		break;
	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : default case not  supported !!! \n", sensor.name, __func__);
		break;
	}
	}

	state->af_mode = ctrl->value;

	return err;
}


static int camdrv_ss_set_AF_default_position(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;


	CAM_INFO_MSG(&client->dev, "%s %s :  value =%d \n", sensor.name, __func__, state->af_mode);

	switch (state->af_mode) {
	case FOCUS_MODE_MACRO:
	{
		if (sensor.af_macro_mode_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : af_macro_mode_regs  supported !!! \n", sensor.name, __func__);
		else
			err |= camdrv_ss_i2c_set_config_register(client, sensor.af_macro_mode_regs, sensor.rows_num_af_macro_mode_regs, "af_macro_mode_regs");

		break;
	}

	case FOCUS_MODE_AUTO:
	{
		if (sensor.af_normal_mode_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : af_normal_mode_regs  supported !!! \n", sensor.name, __func__);
		else
			err |= camdrv_ss_i2c_set_config_register(client, sensor.af_normal_mode_regs, sensor.rows_num_af_normal_mode_regs, "af_normal_mode_regs");

		break;
	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : default case not  supported !!! \n", sensor.name, __func__);
		break;
	}
	}

	return err;
}





static int camdrv_ss_set_af_preflash(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct camdrv_ss_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	if (ctrl->value == PREFLASH_ON) {
		state->camera_af_flash_fire = 0;

		if (state->current_flash_mode == FLASH_MODE_ON) {
			state->camera_af_flash_fire = 1;
		} else if (state->current_flash_mode == FLASH_MODE_AUTO) {
			bool bflash_needed = false;

			if (sensor.check_flash_needed == 0)
				CAM_ERROR_MSG(&client->dev, "%s %s : check_flash_needed NULL!!not  supported !!! \n", sensor.name, __func__);
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
				CAM_ERROR_MSG(&client->dev, "%s %s : snapshot_af_preflash_on_regs  supported !!! \n", sensor.name, __func__);
			else
				err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_af_preflash_on_regs, sensor.rows_num_snapshot_af_preflash_on_regs, "snapshot_af_preflash_on_regs");
			if (err < 0) {
				CAM_ERROR_MSG(&client->dev, "[%s: %d] ERROR! Setting af_preflash_on_regs\n", __FILE__, __LINE__);
			}

			if (sensor.AAT_flash_control == NULL)
				CAM_ERROR_MSG(&client->dev, "%s %s : AAT_flash_control  NULL!!not  supported !!! \n", sensor.name, __func__);
			else
				sensor.AAT_flash_control(sd, FLASH_CONTROL_MIDDLE_LEVEL);
		} else {
			return -1; /* Must return a non-zero value, when flash is not fired. */
		}
	} else { /* if (ctrl->value == PREFLASH_ON) */
		if (state->camera_af_flash_fire) {
			if (sensor.snapshot_af_preflash_off_regs == NULL)
				CAM_ERROR_MSG(&client->dev, "%s %s : snapshot_af_preflash_off_regs  supported !!! \n", sensor.name, __func__);
			else
				err = camdrv_ss_i2c_set_config_register(client, sensor.snapshot_af_preflash_off_regs, sensor.rows_num_snapshot_af_preflash_off_regs, "snapshot_af_preflash_off_regs");
			if (err < 0) {
				CAM_ERROR_MSG(&client->dev, "[%s: %d] ERROR! Setting af_preflash_off_regs\n", __FILE__, __LINE__);
			}

			if (sensor.AAT_flash_control == NULL)
				CAM_ERROR_MSG(&client->dev, "%s %s : AAT_flash_control  NULL!!not  supported !!! \n", sensor.name, __func__);
			else
				sensor.AAT_flash_control(sd, FLASH_CONTROL_OFF);

			state->camera_af_flash_fire = 0;
		}
	}

	return 0;
}


static int camdrv_ss_AE_AWB_lock(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct camdrv_ss_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;
	int rows_num_ = 0;

	CAM_INFO_MSG(&client->dev, "%s %s :  value=%d  \n", sensor.name, __func__, ctrl->value);

	/* Lock, Unlock only AE for LSI 5CC sensor. Don't change AWB. */
	switch (ctrl->value) {
	case AE_UNLOCK_AWB_UNLOCK:
	{
		if (sensor.ae_unlock_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : ae_unlock_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ae_unlock_regs, sensor.rows_num_ae_unlock_regs, "ae_unlock_regs");

		if (sensor.awb_unlock_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : awb_unlock_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.awb_unlock_regs, sensor.rows_num_awb_unlock_regs, "awb_unlock_regs");
	}

	case AE_LOCK_AWB_UNLOCK:
	{
		if (sensor.ae_lock_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : ae_lock_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ae_lock_regs, sensor.rows_num_ae_lock_regs, "ae_lock_regs");

		if (sensor.awb_unlock_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : awb_unlock_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.awb_unlock_regs, sensor.rows_num_awb_unlock_regs, "awb_unlock_regs");
	}

	case AE_UNLOCK_AWB_LOCK:
	{
		if (sensor.ae_unlock_regs == 0)
				CAM_ERROR_MSG(&client->dev, "%s %s : ae_unlock_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ae_unlock_regs, sensor.rows_num_ae_unlock_regs, "ae_unlock_regs");

		if (sensor.awb_lock_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : awb_lock_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.awb_lock_regs, sensor.rows_num_awb_lock_regs, "awb_lock_regs");
	}

	case AE_LOCK_AWB_LOCK:
	{
		if (sensor.ae_lock_regs == 0)
			CAM_ERROR_MSG(&client->dev, "%s %s : ae_lock_regs  supported !!! \n", sensor.name, __func__);
		else
			err = camdrv_ss_i2c_set_config_register(client, sensor.ae_lock_regs, sensor.rows_num_ae_lock_regs, "ae_lock_regs");

		if (!state->camera_af_flash_fire) {
			if (sensor.awb_lock_regs == 0)
				CAM_ERROR_MSG(&client->dev, "%s %s : awb_lock_regs  supported !!! \n", sensor.name, __func__);
			else
				err = camdrv_ss_i2c_set_config_register(client, sensor.awb_lock_regs, sensor.rows_num_awb_lock_regs, "awb_lock_regs");
		}
		break;
	}

	default:
	{
		CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! Unsupported AE, AWB lock setting(%d)\n", __FILE__, __LINE__, ctrl->value);
		break;
	}
	}

	if (err < 0) {
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! AE, AWB lock failed\n", __FILE__, __LINE__);
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
	state->capture_mode = CAMDRV_SS_OP_MODE_VIDEO;
	state->sensor_mode = CAMDRV_SS_OP_MODE_VIDEO;

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
}


/*
 * Clock configuration
 * Configure expected MCLK from host and return EINVAL if not supported clock
 * frequency is expected
 * 	freq : in Hz
 * 	flag : not supported for now
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
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int index = 0;
	state->pix.width = fmt->width;
	state->pix.height = fmt->height;
	state->pix.pixelformat = fmt->code;


	if (state->pix.pixelformat == V4L2_MBUS_FMT_JPEG_1X8 || state->pix.pixelformat == V4L2_PIX_FMT_JPEG) {
		 state->op_mode = CAMDRV_SS_OP_MODE_IMAGE;
		 state->jpeg_param.enable = 1;
		 index =  camdrv_ss_find_capture_framesize(state->pix.width , state->pix.height);
		 fmt->width = state->pix.width = sensor.supported_capture_framesize_list[index].width;
		 fmt->height = state->pix.height = sensor.supported_capture_framesize_list[index].height;
		 state->capture_framesize_index = sensor.supported_capture_framesize_list[index].index;
		 CAM_INFO_MSG(&client->dev, "%s %s :  capture_framesize_index=%d  \n", sensor.name, __func__, state->capture_framesize_index);

	} else {
		state->op_mode = CAMDRV_SS_OP_MODE_VIDEO;
		state->jpeg_param.enable = 0;

		index =  camdrv_ss_find_preview_framesize(state->pix.width , state->pix.height);
		fmt->width = state->pix.width = sensor.supported_preview_framesize_list[index].width;
		fmt->height = state->pix.height = sensor.supported_preview_framesize_list[index].height;
		state->preview_framesize_index = sensor.supported_preview_framesize_list[index].index;

		index =  camdrv_ss_find_capture_framesize(state->pix.width , state->pix.height);
		state->capture_framesize_index = sensor.supported_capture_framesize_list[index].index;
		CAM_INFO_MSG(&client->dev, "%s %s : preview_framesize_index=%d  capture_framesize_index = %d\n", sensor.name, __func__, state->preview_framesize_index, state->capture_framesize_index);

	}
	return 0;
}


static int camdrv_ss_enum_framesizes(struct v4l2_subdev *sd, struct v4l2_frmsizeenum *fsize)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	CAM_INFO_MSG(&client->dev, "%s %s :  \n", sensor.name, __func__);

	if (fsize->pixel_format == sensor.default_pix_fmt || fsize->pixel_format == sensor.default_mbus_pix_fmt) {
		if (fsize->index >= sensor.supported_number_of_preview_sizes) {
			CAM_ERROR_MSG(&client->dev, "%s %s : exceeded index =%d  \n", sensor.name, __func__, fsize->index);
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
			CAM_ERROR_MSG(&client->dev, "%s %s : exceeded index =%d  \n", sensor.name, __func__, fsize->index);
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
		CAM_ERROR_MSG(&client->dev, "%s %s : wrong format =%d ,failed !! \n", sensor.name, __func__, fsize->index);
		return -EINVAL;
	}

	return 0;
}


static int camdrv_ss_enum_frameintervals(struct v4l2_subdev *sd,
					struct v4l2_frmivalenum *fival)
{
	printk("camdrv_ss_enum_frameintervals w = %d h = %d \n", fival->width, fival->height);
	return sensor.enum_frameintervals(sd, fival);
}


static int camdrv_ss_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
				enum v4l2_mbus_pixelcode *code)
{
	int num_entries = sensor.rows_num_fmts;
	if (index >= num_entries) {
		CAM_ERROR_MSG(&client->dev, "%s %s : index =%d, num_entries =%d failed \n", sensor.name, __func__, index, num_entries);
		return -EINVAL;
	}

	*code = sensor.fmts[index].pixelformat;
	return 0;
}


static int camdrv_ss_try_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	int num_entries;
	int i;

	num_entries = sensor.rows_num_fmts;

	for (i = 0; i < num_entries; i++) {
		if (sensor.fmts[i].pixelformat == mf->code) {
			CAM_INFO_MSG(&client->dev, "%s %s : match found for =%d !! \n", sensor.name, __func__, mf->code);
			return 0;
		}
	}

	CAM_ERROR_MSG(&client->dev, "%s %s : match not found for =%d  failed  !! \n", sensor.name, __func__, mf->code);
}


/* Gets current FPS value */
static int camdrv_ss_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct camdrv_ss_state *state = to_state(sd);

	int err = 0;

	state->strm.parm.capture.timeperframe.numerator = 1;
	state->strm.parm.capture.timeperframe.denominator = state->fps;

	memcpy(param, &state->strm, sizeof(param));

	return err;
}


/* Sets the FPS value */
static int camdrv_ss_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct camdrv_ss_state *state = to_state(sd);

	state->capture_mode = param->parm.capture.capturemode;

	return 0;
}


static int camdrv_ss_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	struct camdrv_ss_userset userset = state->userset;
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s %s : id = %d !! \n", sensor.name, __func__, ctrl->id);

	switch (ctrl->id) {
	case V4L2_CID_EXPOSURE:
	{
		ctrl->value = userset.exposure_bias;
		break;
	}

	case V4L2_CID_AUTO_WHITE_BALANCE:
	{
		ctrl->value = userset.auto_wb;
		break;
	}

	case V4L2_CID_WHITE_BALANCE_PRESET:
	{
		ctrl->value = userset.manual_wb;
		break;
	}

	case V4L2_CID_COLORFX:
	{
		ctrl->value = userset.effect;
		break;
	}

	case V4L2_CID_CONTRAST:
	{
		ctrl->value = userset.contrast;
		break;
	}

	case V4L2_CID_SATURATION:
	{
		ctrl->value = userset.saturation;
		break;
	}

	case V4L2_CID_SHARPNESS:
	{
		ctrl->value = userset.sharpness;
		break;
	}

	case V4L2_CID_CAM_JPEG_MAIN_SIZE:
	{
		ctrl->value = state->jpeg_param.main_size;
		break;
	}

	case V4L2_CID_CAM_JPEG_MAIN_OFFSET:
	{
		ctrl->value = state->jpeg_param.main_offset;
		break;
	}

	case V4L2_CID_CAM_JPEG_THUMB_SIZE:
	{
		ctrl->value = state->jpeg_param.thumb_size;
		break;
	}

	case V4L2_CID_CAM_JPEG_THUMB_OFFSET:
	{
		ctrl->value = state->jpeg_param.thumb_offset;
		break;
	}

	case V4L2_CID_CAM_JPEG_POSTVIEW_OFFSET:
	{
		ctrl->value = state->jpeg_param.postview_offset;
		break;
	}

	case V4L2_CID_CAM_JPEG_MEMSIZE:
	{
		/* ctrl->value = SENSOR_JPEG_SNAPSHOT_MEMSIZE; */
		break;
	}

	case V4L2_CID_CAM_JPEG_QUALITY:
	{
		ctrl->value = state->jpeg_param.quality;
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
		if (state->bTouchFocus)
			err = sensor.get_touch_focus_status(sd, ctrl);
		else
			err = sensor.get_auto_focus_status(sd, ctrl);

		break;
	}

	case V4L2_CID_CAMERA_AE_STABLE_RESULT:
	{
		err = sensor.get_ae_stable_status(sd, ctrl);
		break;
	}

	case V4L2_CID_CAM_DATE_INFO_YEAR:
	{
		ctrl->value = 2010; /* state->dateinfo.year; //bestiq */
		break;
	}

	case V4L2_CID_CAM_DATE_INFO_MONTH:
	{
		ctrl->value = 2; /* state->dateinfo.month; */
		break;
	}

	case V4L2_CID_CAM_DATE_INFO_DATE:
	{
		ctrl->value = 25; /* state->dateinfo.date; */
		break;
	}

	case V4L2_CID_CAM_SENSOR_VER:
	{
		ctrl->value = state->camdrv_ss_version;
		break;
	}

	case V4L2_CID_CAM_FW_MINOR_VER:
	{
		ctrl->value = state->fw_ver.minor;
		break;
	}

	case V4L2_CID_CAM_FW_MAJOR_VER:
	{
		ctrl->value = state->fw_ver.major;
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
		ctrl->value = state->camera_flash_fire;
		break;
	}

	case V4L2_CID_CAMERA_POSTVIEW_WIDTH:
	{
		ctrl->value = state->postview_info.width;
		break;
	}

	case V4L2_CID_CAMERA_POSTVIEW_HEIGHT:
	{
		ctrl->value = state->postview_info.height;
		break;
	}

	case V4L2_CID_CAMERA_SENSOR_ID:
	{
		/* ctrl->value = SENSOR_ID; */
		break;
	}

	case V4L2_CID_CAMERA_GET_ISO:
	{
		ctrl->value = state->isoSpeedRating;
		break;
	}

	case V4L2_CID_CAMERA_GET_SHT_TIME:
	{
		ctrl->value = state->exposureTime;
		break;
	}

	default:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : default control id =%d failed !! \n", sensor.name, __func__, ctrl->id);
		return -ENOIOCTLCMD;
	}
	}

	return err;
}


static int camdrv_ss_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camdrv_ss_state *state = to_state(sd);
	int err = 0;

	if (!bCameraInitialized) {
		CAM_ERROR_MSG(&client->dev, "%s %s :camera not yet initialized!! \n", sensor.name, __func__);
		return 0;
	}
	CAM_ERROR_MSG(&client->dev, "%s %s : id = %d !! \n", sensor.name, __func__, ctrl->id);

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
		break;
	}

	case V4L2_CID_CAMERA_AE_AWB_LOCKUNLOCK:
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
		if (sensor.AAT_flash_control != NULL)
			err = sensor.AAT_flash_control(sd, ctrl->value);
		break;
	}

	case V4L2_CID_CAMERA_BRIGHTNESS:
	{
		err = camdrv_ss_set_ev(sd, ctrl);
		break;
	}

	case V4L2_CID_CAMERA_WHITE_BALANCE:
	{
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
		err = camdrv_ss_set_iso(sd, ctrl);
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

	/* Camcorder fix fps */
	case V4L2_CID_CAMERA_SENSOR_MODE:
	{
		state->sensor_mode = ctrl->value;

		break;
	}

	case V4L2_CID_CAMERA_WDR:
	{
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
		err = camdrv_ss_set_focus_mode(sd, ctrl);
		break;
	}

	case V4L2_CID_CAMERA_DEFAULT_FOCUS_POSITION:
	{
		err = camdrv_ss_set_AF_default_position(sd);
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
		if (ctrl->value < 0 || ctrl->value > 100) {
			err = -EINVAL;
		} else {
			state->jpeg_param.quality = ctrl->value;
			err = camdrv_ss_set_jpeg_quality(sd);
		}
		break;
	}

	case V4L2_CID_CAMERA_SCENE_MODE:
	{
		err = camdrv_ss_set_scene_mode(sd, ctrl);
		break;
	}

	case V4L2_CID_CAMERA_GPS_LATITUDE:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : V4L2_CID_CAMERA_GPS_LATITUDE not implemented !! \n", sensor.name, __func__);
		break;
	}

	case V4L2_CID_CAMERA_GPS_LONGITUDE:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : V4L2_CID_CAMERA_GPS_LONGITUDE not implemented !! \n", sensor.name, __func__);
		break;
	}

	case V4L2_CID_CAMERA_GPS_TIMESTAMP:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : V4L2_CID_CAMERA_GPS_TIMESTAMP not implemented !! \n", sensor.name, __func__);
		break;
	}

	case V4L2_CID_CAMERA_GPS_ALTITUDE:
	{
		CAM_ERROR_MSG(&client->dev, "%s %s : V4L2_CID_CAMERA_GPS_ALTITUDE not implemented !! \n", sensor.name, __func__);
		break;
	}

	case V4L2_CID_CAMERA_ZOOM:
	{
		err = camdrv_ss_set_dzoom(sd, ctrl);
		printk(KERN_NOTICE "BILLA V4L2_CID_CAMERA_ZOOM success\n");
		break;
	}

	case V4L2_CID_CAMERA_TOUCH_AF_AREA:
	{
		v4l2_touch_area touch_area;
		copy_from_user(&touch_area,
			(v4l2_touch_area *)ctrl->value,
			sizeof(v4l2_touch_area));

		state->bTouchFocus = true;
		state->touch_area.x = touch_area.x;
		state->touch_area.y = touch_area.y;
		state->touch_area.w = touch_area.w;
		state->touch_area.h = touch_area.h;
		state->touch_area.weight = touch_area.weight;
		printk(" V4L2_CID_CAMERA_TOUCH_AF_AREA x =%d, y =%d ,w = %d, h =%d, weight=%d\n",
			state->touch_area.x,
			state->touch_area.y,
			state->touch_area.w,
			state->touch_area.h,
			state->touch_area.weight);
	}

	case V4L2_CID_CAMERA_CAF_START_STOP:
	{
		break;
	}

	case V4L2_CID_CAMERA_OBJECT_POSITION_X:
	{
		state->af_info.x = ctrl->value;
		err = 0;
		break;
	}

	case V4L2_CID_CAMERA_OBJECT_POSITION_Y:
	{
		state->af_info.y = ctrl->value;
		err = 0;
		break;
	}

	case V4L2_CID_CAMERA_OBJ_TRACKING_START_STOP:
	{
		break;
	}

	case V4L2_CID_CAMERA_SET_AUTO_FOCUS:
	{
		printk(" V4L2_CID_CAMERA_SET_AUTO_FOCUS %d\n", ctrl->value);
		if (state->bTouchFocus) {
			if (ctrl->value == AUTO_FOCUS_ON) {
				err = sensor.set_touch_focus(sd, TOUCH_AF_START, &(state->touch_area));
			} else {
				err = sensor.set_touch_focus(sd, TOUCH_AF_STOP, NULL);
				state->bTouchFocus = false;
			}
		} else {
			err = sensor.set_auto_focus(sd, ctrl);
		}
		break;
	}

	case V4L2_CID_CAMERA_SET_AF_PREFLASH:
	{
		err = camdrv_ss_set_af_preflash(sd, ctrl);
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

	case V4L2_CID_CAM_CAPTURE:
	{
		err = camdrv_ss_set_capture_start(sd, ctrl);
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
		if (ctrl->value) {
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
		state->check_dataline = ctrl->value;
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

	default:
	{
		err = -ENOTSUPP;
		break;
	}
	}

	if (err < 0) {
	   CAM_ERROR_MSG(&client->dev, "%s %s : ERROR for id  =%d !! \n", sensor.name, __func__, ctrl->id);
	}

	mutex_unlock(&sensor_s_ctrl);
	CAM_ERROR_MSG(&client->dev, "%s %s : %d SUCCESS  \n", sensor.name, __func__, ctrl->id);
	return err;
}


static int camdrv_ss_init(struct v4l2_subdev *sd, u32 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;


	CAM_INFO_MSG(&client->dev, "%s %s : \n", sensor.name, __func__);

	camdrv_ss_init_parameters(sd);

#ifdef CONFIG_LOAD_FILE
	err = camdrv_ss_regs_table_init(sd);
	if (err < 0) {
		CAM_ERROR_MSG(&client->dev, "%s: camdrv_ss_regs_table_init failed\n", __func__);
		return -ENOIOCTLCMD;
	}
#endif /* CONFIG_LOAD_FILE */
	CAM_INFO_MSG(&client->dev, "%s %s : rows = %d \n", sensor.name, __func__, sensor.rows_num_init_regs);
	if (sensor.init_regs == NULL)
		CAM_ERROR_MSG(&client->dev, "%s %s : init_regs is NULL, please check if it is needed !!! \n", sensor.name, __func__);
	else
		err = camdrv_ss_i2c_set_config_register(client, sensor.init_regs, sensor.rows_num_init_regs, "init_regs");

	if (err < 0) {
		CAM_INFO_MSG(&client->dev, "%s %s :i2c failed !! \n", sensor.name, __func__);
		return -EINVAL;
	}

	return 0;
}

static struct pin_config GPIOSetup;
/**************************************************************************
 * DRIVER REGISTRATION FACTORS
 ***************************************************************************/


static int camdrv_ss_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct camdrv_ss_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;


	CAM_INFO_MSG(&client->dev, "%s %s : \n", sensor.name, __func__);
	if (bCaptureMode == true) {
		CAM_INFO_MSG(&client->dev, "%s %s in capture mode, ignore this function call: \n", sensor.name, __func__);
		return 0;
	}

	if (enable) {
		if (bCameraPowerUp == false) {
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

			sensor.sensor_power(1);
			bCameraPowerUp = true;
		}

		if (bCameraInitialized == false) {
			CAM_INFO_MSG(&client->dev, "%s %s : Initializing camera.... \n", sensor.name, __func__);
			err = camdrv_ss_init(sd, 1);
			if (err < 0) {
				CAM_INFO_MSG(&client->dev, "camdrv_ss_init failed !! \n");
				return -EINVAL;
			}
			bCameraInitialized = true;
		}

		if (state->pix.pixelformat == sensor.default_mbus_pix_fmt
			|| state->pix.pixelformat == sensor.default_pix_fmt) {
			CAM_INFO_MSG(&client->dev, "%s %s : PREVIEW MODE.. \n", sensor.name, __func__);
			camdrv_ss_set_preview_start(sd);
		} else {
			CAM_INFO_MSG(&client->dev, "%s %s : CAPTURE MODE.. \n", sensor.name, __func__);
			camdrv_ss_set_capture_start(sd, NULL);
		}
		CAM_INFO_MSG(&client->dev, "%s %s : START SUCCESS!! \n", sensor.name, __func__);
	} else {
		camdrv_ss_set_preview_stop(sd);
		CAM_INFO_MSG(&client->dev, "%s : STOP success \n", __func__);
	}

	/* printk(KERN_NOTICE "Error !!!!, camdrv_ss_s_stream is empty\n"); */
	return 0;
}


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


static long camdrv_ss_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	sensor.thumbnail_ioctl(sd, cmd, arg);
}


static int camdrv_ss_enum_input(struct soc_camera_device *icd, struct v4l2_input *inp)
{
	struct soc_camera_link *icl = to_soc_camera_link(icd);
	struct v4l2_subdev_sensor_interface_parms *plat_parms;

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->std  = V4L2_STD_UNKNOWN;
	if (camera_id == CAMDRV_SS_CAM_ID_MAIN)
		strcpy(inp->name, CAMDRV_SS_MODULE_NAME_MAIN);
	else
		strcpy(inp->name, CAMDRV_SS_MODULE_NAME_SUB);

	if (icl && icl->priv) {
		plat_parms = icl->priv;
		inp->status = 0;

		if (plat_parms->orientation == V4L2_SUBDEV_SENSOR_PORTRAIT)
			inp->status |= V4L2_IN_ST_HFLIP;

		if (plat_parms->facing == V4L2_SUBDEV_SENSOR_BACK)
			inp->status |= V4L2_IN_ST_BACK;
	}
	return 0;
}


static struct soc_camera_ops camdrv_ss_ops = {
	.set_bus_param		= camdrv_ss_set_bus_param,
	.query_bus_param	= camdrv_ss_query_bus_param,
	.enum_input		= camdrv_ss_enum_input,
	/* .controls			= controls, */
	/* .num_controls		= ARRAY_SIZE(*controls), */
};


static const struct v4l2_subdev_core_ops camdrv_ss_core_ops = {
	.init		= camdrv_ss_init, /* initializing API */
	/* .s_config	= camdrv_ss_s_config, */ /* Fetch platform data */
	/* .queryctrl	= camdrv_ss_queryctrl, */
	/* .querymenu	= camdrv_ss_querymenu, */
	.g_ctrl		= camdrv_ss_g_ctrl,
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
	/* Quantity of initial bad frames to skip. Revisit. */
	*frames = 0;

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
		CAM_PRINTK(KERN_ERR "[!!WARN!!]%s: hs_term_time is default value(%d)\n",
			sensor.name,
			parms->parms.serial.hs_term_time);
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
		printk("%s camera id =%d is WRONG !!!\n", __func__, cam_id);
		return false;
	}
	camera_id = cam_id;

	if (bOn) {
		memset(&sensor, 0, sizeof(struct camdrv_ss_sensor_cap));
		printk("%s cam_id =%d ON \n", __func__, cam_id);
		if (camera_id == CAMDRV_SS_CAM_ID_MAIN)
			camdrv_ss_sensor_init_main(bOn, &sensor);
#ifdef CONFIG_SOC_SUB_CAMERA
		else if (camera_id == CAMDRV_SS_CAM_ID_SUB)
			camdrv_ss_sensor_init_sub(bOn, &sensor);
#endif

		if (sensor.thumbnail_ioctl == NULL) {
			CAM_INFO_MSG(&client->dev, "%s %s : sensor.thumbnail_ioctl == NULL failed !\n", __func__, sensor.name);
			return false;
		}

		if (sensor.controls == NULL) {
			CAM_INFO_MSG(&client->dev, "%s %s : sensor.controls == NULL failed !\n", __func__, sensor.name);
			return false;
		}

		if (sensor.enum_frameintervals == NULL) {
			CAM_INFO_MSG(&client->dev, "%s %s : sensor.enum_frameintervals == NULL failed !\n", __func__, sensor.name);
			return false;
		}

		camdrv_ss_ops.controls = sensor.controls;
		camdrv_ss_ops.num_controls = sensor.rows_num_controls;
	} else {
		printk("%s cam_id =%d OFF\n", __func__, cam_id);
		bCameraInitialized = false;
		if (bCameraPowerUp == true) {
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
			bCameraPowerUp = false;
		}
		memset(&sensor, 0, sizeof(struct camdrv_ss_sensor_cap));
	}

	return true;
}


/**************************************************************************
 * camdrv_ss_probe
 * Fetching platform data is being done with s_config sd call.
 * In probe routine, we just register sd device
 ***************************************************************************/
static int camdrv_ss_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct camdrv_ss_state *state;
	struct v4l2_subdev *sd;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl = to_soc_camera_link(icd);

	CAM_INFO_MSG(&client->dev, "%s %s : \n", __func__, sensor.name);

	if (!icd) {
		CAM_ERROR_MSG(&client->dev, "%s %s : icd NULL failed !!\n", __func__, sensor.name);
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		CAM_ERROR_MSG(&client->dev, "%s %s : icl NULL failed !!\n", __func__, sensor.name);
		return -EINVAL;
	}

	if (!icl->priv) {
		CAM_ERROR_MSG(&client->dev, "%s %s : icl->priv is NULL failed !!\n", __func__, sensor.name);
		return -EINVAL;
	}

	state = kzalloc(sizeof(struct camdrv_ss_state), GFP_KERNEL);
	if (state == NULL) {
		CAM_ERROR_MSG(&client->dev, "%s %s : alloc failed !!\n", __func__, sensor.name);
		return -ENOMEM;
	}

	state->runmode = CAMDRV_SS_RUNNING_MODE_NOTREADY;
	state->af_mode = 0;
	state->currentScene = SCENE_MODE_NONE;
	state->currentWB = WHITE_BALANCE_AUTO;
	state->currentMetering = METERING_CENTER;
	state->bStartFineSearch  = false;
	state->isoSpeedRating = 0;
	state->exposureTime = 0;

	sd = &state->sd;
	if (camera_id == CAMDRV_SS_CAM_ID_MAIN)
		strcpy(sd->name, CAMDRV_SS_MODULE_NAME_MAIN);
	else
		strcpy(sd->name, CAMDRV_SS_MODULE_NAME_SUB);

	/* strcpy(sd->name, CAMDRV_SS_MODULE_NAME_MAIN); */
	CAM_INFO_MSG(&client->dev, "%s %s : \n", __func__, icl->module_name);

	state->plat_parms = icl->priv;

	/* Registering sd */
	v4l2_i2c_subdev_init(sd, client, &camdrv_ss_subdev_ops);
	icd->ops		= &camdrv_ss_ops;

#ifdef FACTORY_CHECK
	{
		if (sec_cam_dev == NULL) {
			sec_cam_dev = device_create(sec_class, NULL, 0, NULL, "sec_cam");
			if (IS_ERR(sec_cam_dev))
				pr_err("Failed to create device(sec_cam_dev)!\n");
		}

		if (sec_cam_dev != NULL && camtype_init == false) {
			camtype_init = true;
			if (device_create_file(sec_cam_dev, &dev_attr_camtype) < 0)
				pr_err("Failed to create device file(%s)!\n", dev_attr_camtype.attr.name);
		}
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

	CAM_INFO_MSG(&client->dev, "%s %s : success\n", sensor.name, __func__);

	return 0;
}


/**************************************************************************
 * camdrv_ss_remove
 ***************************************************************************/
static int camdrv_ss_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl = to_soc_camera_link(icd);

	CAM_ERROR_MSG(&client->dev, "%s %s : \n", sensor.name, __func__);

#ifdef CONFIG_LOAD_FILE
	camdrv_ss_regs_table_exit();
#endif /* CONFIG_LOAD_FILE */

	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));

#ifdef FACTORY_CHECK
	if (sec_cam_dev != NULL && camtype_init == true) {
		camtype_init = false;
		device_remove_file(sec_cam_dev, &dev_attr_camtype);
	}

	if (sec_cam_dev != NULL) {
		device_destroy(sec_class, 0);
	}
#endif

	CAM_ERROR_MSG(&client->dev, "%s %s : unloaded \n", sensor.name, __func__);

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
	CAM_INFO_MSG(&client->dev, "%s %s : \n", CAMDRV_SS_MODULE_NAME_MAIN, __func__);
	i2c_add_driver(&camdrv_ss_i2c_driver);

#ifdef CONFIG_SOC_SUB_CAMERA
	return i2c_add_driver(&camdrv_ss_i2c_driver_sub);
#endif
}

static void __exit camdrv_ss_mod_exit(void)
{
	i2c_del_driver(&camdrv_ss_i2c_driver);

#ifdef CONFIG_SOC_SUB_CAMERA
	i2c_del_driver(&camdrv_ss_i2c_driver_sub);
#endif
}


module_init(camdrv_ss_mod_init);
module_exit(camdrv_ss_mod_exit);


MODULE_DEVICE_TABLE(i2c, camdrv_ss_id);
MODULE_DESCRIPTION("SAMSUNG CAMERA SENSOR DRIVER ");
MODULE_AUTHOR("Samsung");
MODULE_LICENSE("GPL");

