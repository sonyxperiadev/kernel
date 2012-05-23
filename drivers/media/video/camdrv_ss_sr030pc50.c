

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
#include <camdrv_ss.h>
#include <camdrv_ss_sr030pc50.h>


#define SR030PC50_NAME	"sr030pc50"

#define SR030PC50_DEFAULT_PIX_FMT		V4L2_PIX_FMT_UYVY	/* YUV422 */
#define SR030PC50_DEFAULT_MBUS_PIX_FMT    V4L2_MBUS_FMT_UYVY8_2X8
#define SR030PC50_REGISTER_SIZE 2



static const struct camdrv_ss_framesize sr030pc50_supported_preview_framesize_list[] = {
	{ PREVIEW_SIZE_VGA,	640,  480 },
};

static const struct camdrv_ss_framesize  sr030pc50_supported_capture_framesize_list[] = {
	{ CAPTURE_SIZE_VGA, 640,  480 }
};

const static struct v4l2_fmtdesc sr030pc50_fmts[] = {
	{
	.index		= 0,
	.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE,
	.flags		= 0,
	.description	= "UYVY",
	.pixelformat	= V4L2_MBUS_FMT_UYVY8_2X8,
	},
};

static const struct v4l2_queryctrl sr030pc50_controls[] = {
	{
	},
};


static int camdrv_ss_sr030pc50_enum_frameintervals(struct v4l2_subdev *sd, struct v4l2_frmivalenum *fival)
{
	int err = 0;
	int size, i;

	if (fival->index >= 1)
		return -EINVAL;

	printk(KERN_INFO " %s :  E\n", __func__);

	fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	for (i = 0; i < ARRAY_SIZE(sr030pc50_supported_preview_framesize_list); i++) {
		if ((sr030pc50_supported_preview_framesize_list[i].width == fival->width) &&
		    (sr030pc50_supported_preview_framesize_list[i].height == fival->height)) {
			size = sr030pc50_supported_preview_framesize_list[i].index;
			break;
		}
	}
	if (i == ARRAY_SIZE(sr030pc50_supported_preview_framesize_list)) {
		printk(KERN_INFO "%s unsupported width = %d and height = %d\n",
			__func__, fival->width, fival->height);
		return -EINVAL;
	}

	switch (size) {
	case PREVIEW_SIZE_VGA:
		fival->discrete.numerator = 1;
		fival->discrete.denominator = 30;
		break;
	default:
		fival->discrete.numerator = 1;
		fival->discrete.denominator = 30;
		break;
	}

	return err;
}



static long sr030pc50_ss_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
/*s	struct i2c_client *client = v4l2_get_subdevdata(sd); */
/*	struct camdrv_ss_state *state = */
/*	container_of(sd, struct camdrv_ss_states, sd); */
	int ret = 0;

	switch (cmd) {

	case VIDIOC_THUMB_SUPPORTED:
	{
		int *p = arg;
		*p = 0; /* NO */

		break;
	}

	case VIDIOC_THUMB_G_FMT:
	{
		struct v4l2_format *p = arg;
		struct v4l2_pix_format *pix = &p->fmt.pix;
		p->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		/* fixed thumbnail resolution and format */
		pix->width = 640;
		pix->height = 480;
		pix->bytesperline = 640 * 2;
		pix->sizeimage = 640 * 480 * 2;
		pix->field = V4L2_FIELD_ANY;
		pix->colorspace = V4L2_COLORSPACE_JPEG,
		pix->pixelformat = V4L2_PIX_FMT_UYVY;
		break;
	}

	case VIDIOC_THUMB_S_FMT:
	{
		/*	struct v4l2_format *p = arg; */
		/* for now we don't support setting thumbnail fmt and res */
		ret = -EINVAL;
		break;
	}

	case VIDIOC_JPEG_G_PACKET_INFO:
	{
		struct v4l2_jpeg_packet_info *p = arg;
		p->padded = 0;
		p->packet_size = 0;
		break;
	}

	default:
		ret = -ENOIOCTLCMD;
		break;
	}

	return ret;
}

#ifdef CONFIG_MACH_RHEA_SS_AMAZING
/* Power (common) */
static struct regulator *VCAM_IO_1_8_V;  /* LDO_HV9 */
static struct regulator *VCAM_A_2_8_V;   /* LDO_CAM12/12/2011 */
#define CAM_CORE_EN                  42
/* #define CAM_AF_EN     121 */

/* main cam  */
#define CAM0_RESET    33
#define CAM0_STNBY    111

/* sub cam */
#define CAM1_RESET    23
#define CAM1_STNBY    34

#define SENSOR_0_CLK			"dig_ch0_clk"    /* (common) */
#define SENSOR_0_CLK_FREQ		(26000000) /* @HW, need to check how fast this meaning. */

static int sr030pc50_sensor_power(int on)
{
	unsigned int value;
	int ret = -1;
	struct clk *clock;
	struct clk *axi_clk;
	static struct pi_mgr_dfs_node unicam_dfs_node;

	printk(KERN_INFO "%s:camera power %s\n", __func__, (on ? "on" : "off"));

	if (!unicam_dfs_node.valid) {
		ret = pi_mgr_dfs_add_request(&unicam_dfs_node, "unicam", PI_MGR_PI_ID_MM,
					   PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			printk(KERN_ERR
				"%s: failed to register PI DFS request\n",
				__func__);
			return -1;
		}
	}
	clock = clk_get(NULL, SENSOR_0_CLK);
	if (!clock) {
		printk(KERN_ERR "%s: unable to get clock %s\n", __func__, SENSOR_0_CLK);
		return -1;
	}
	axi_clk = clk_get(NULL, "csi0_axi_clk");
	if (!axi_clk) {
		printk(KERN_ERR "%s:unable to get clock csi0_axi_clk\n", __func__);
		return -1;
	}
	VCAM_A_2_8_V = regulator_get(NULL, "cam");
	if (IS_ERR(VCAM_A_2_8_V)) {
		printk(KERN_INFO "can not get VCAM_A_2_8_V.8V\n");
		return -1;
	}
	regulator_set_voltage(VCAM_A_2_8_V, 2800000, 2800000);
	ret = regulator_enable(VCAM_A_2_8_V);

	VCAM_IO_1_8_V = regulator_get(NULL, "hv9");
	if (IS_ERR(VCAM_IO_1_8_V)) {
		printk(KERN_INFO "can not get VCAM_IO_1.8V\n");
		return -1;
	}
	regulator_set_voltage(VCAM_IO_1_8_V, 1800000, 1800000);
	ret = regulator_enable(VCAM_IO_1_8_V);


	gpio_request(CAM_CORE_EN, "cam_1_2v");
	gpio_direction_output(CAM_CORE_EN, 1);

	/* gpio_request(CAM_AF_EN, "cam_af_2_8v"); */
	/* gpio_direction_output(CAM_AF_EN,0);  */

	printk(KERN_INFO "set cam_rst cam_stnby  to low\n");
	gpio_request(CAM0_RESET, "cam0_rst");
	gpio_direction_output(CAM0_RESET, 0);

	gpio_request(CAM0_STNBY, "cam0_stnby");
	gpio_direction_output(CAM0_STNBY, 0);

	regulator_disable(VCAM_A_2_8_V);
	regulator_disable(VCAM_IO_1_8_V);
	gpio_direction_output(CAM_CORE_EN, 0);

	gpio_request(CAM1_RESET, "cam1_rst");
	gpio_direction_output(CAM1_RESET, 0);

	gpio_request(CAM1_STNBY, "cam1_stnby");
	gpio_direction_output(CAM1_STNBY, 0);

/*	value = ioread32(padctl_base + PADCTRLREG_DCLK1_OFFSET) & (~PADCTRLREG_DCLK1_PINSEL_DCLK1_MASK); */
/*		iowrite32(value, padctl_base + PADCTRLREG_DCLK1_OFFSET); */


	if (on) {
		if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_TURBO)) {
			printk(KERN_ERR
				"%s:failed to update dfs request for unicam\n",
				__func__);
			return -1;
		}

		value = clk_enable(axi_clk);
		if (value) {
			printk(KERN_ERR "%s:failed to enable csi2 axi clock\n", __func__);
			return -1;
		}

		msleep(100);
		printk(KERN_INFO "power on the sensor's power supply\n"); /* @HW */

		VCAM_A_2_8_V = regulator_get(NULL, "cam");
		if (IS_ERR(VCAM_A_2_8_V)) {
			printk(KERN_INFO "can not get VCAM_A_2_8_V.8V\n");
			return -1;
		}
		regulator_set_voltage(VCAM_A_2_8_V, 2800000, 2800000);

		regulator_enable(VCAM_A_2_8_V);
		msleep(1);

		VCAM_IO_1_8_V = regulator_get(NULL, "hv9");
		if (IS_ERR(VCAM_IO_1_8_V)) {
			printk(KERN_INFO "can not get VCAM_IO_1.8V\n");
			return -1;
		}
		regulator_set_voltage(VCAM_IO_1_8_V, 1800000, 1800000);
		regulator_enable(VCAM_IO_1_8_V);

		msleep(5);

		value = clk_enable(clock);
		if (value) {
			printk(KERN_ERR "%s: failed to enable clock %s\n", __func__,
				SENSOR_0_CLK);
			return -1;
		}
		printk(KERN_INFO "enable camera clock\n");
		value = clk_set_rate(clock, SENSOR_0_CLK_FREQ);
		if (value) {
			printk(KERN_ERR "%s: failed to set the clock %s to freq %d\n",
					__func__, SENSOR_0_CLK, SENSOR_0_CLK_FREQ);
			return -1;
		}
		printk(KERN_INFO "set rate\n");
		msleep(5);

		gpio_set_value(CAM1_STNBY, 1);
		msleep(35);

		gpio_set_value(CAM1_RESET, 1);
		msleep(2);

		gpio_set_value(CAM1_STNBY, 0);
		msleep(12);

		/*Main Hi-Z*/
		gpio_set_value(CAM0_STNBY, 1);
		msleep(1);

		gpio_set_value(CAM0_RESET, 1);
		msleep(1);

		gpio_set_value(CAM0_STNBY, 0);
		msleep(1);

		gpio_set_value(CAM1_STNBY, 1);
		msleep(12);

		gpio_set_value(CAM1_RESET, 0);
		msleep(1);
		gpio_set_value(CAM1_RESET, 1);

		printk(KERN_INFO "set cam rst to high\n");
		msleep(50);
	} else {
		/* enable reset gpio */
		gpio_set_value(CAM1_RESET, 0);
		msleep(1);

		clk_disable(clock);
		clk_disable(axi_clk);
		msleep(1);

		gpio_set_value(CAM1_STNBY, 0);
		msleep(1);
		gpio_set_value(CAM0_STNBY, 0);
		msleep(1);

		gpio_set_value(CAM0_RESET, 0);

		/* enable power down gpio */
		regulator_disable(VCAM_IO_1_8_V);
		regulator_disable(VCAM_A_2_8_V);

		if (pi_mgr_dfs_request_update(&unicam_dfs_node,
					      PI_MGR_DFS_MIN_VALUE)) {
			printk(KERN_ERR "%s: failed to update dfs request for unicam\n",
				 __func__);
		}
	}

	return 0;
}

#elif CONFIG_MACH_RHEA_SS_LUCAS

/* @HW */
/* Power (common) */
static struct regulator *VCAM_IO_1_8_V;  /* LDO_HV9 */
static struct regulator *VCAM_A_2_8_V;   /* LDO_CAM12/12/2011 */
#define CAM_CORE_EN	   42
#define CAM_AF_EN     121

/* main cam  */
#define CAM0_RESET    33
#define CAM0_STNBY    111

/* sub cam */
#define CAM1_RESET    23
#define CAM1_STNBY    34

#define SENSOR_0_CLK			"dig_ch0_clk"    /* (common) */
#define SENSOR_0_CLK_FREQ		(26000000) /* @HW, need to check how fast this meaning. */

/* flash */
/* #define	CAM_FLASH_MODE  */
/* #define       CAM_FLASH_EN   */

static int sr030pc50_sensor_power(struct device *dev, int on)
{
	unsigned int value;
	int ret = -1;
	struct clk *clock;
	struct clk *axi_clk;
	static struct pi_mgr_dfs_node unicam_dfs_node;

	printk(KERN_INFO "%s:camera power %s\n", __func__, (on ? "on" : "off"));

	if (!unicam_dfs_node.valid) {
		ret = pi_mgr_dfs_add_request(&unicam_dfs_node, "unicam", PI_MGR_PI_ID_MM,
					   PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			printk(KERN_ERR
				"%s: failed to register PI DFS request\n",
				__func__);
			return -1;
		}
	}
	clock = clk_get(NULL, SENSOR_0_CLK);
	if (!clock) {
		printk(KERN_ERR "%s: unable to get clock %s\n", __func__, SENSOR_0_CLK);
		return -1;
	}
	axi_clk = clk_get(NULL, "csi0_axi_clk");
	if (!axi_clk) {
		printk(KERN_ERR "%s:unable to get clock csi0_axi_clk\n", __func__);
		return -1;
	}
	VCAM_A_2_8_V = regulator_get(NULL, "cam");
	if (IS_ERR(VCAM_A_2_8_V)) {
		printk(KERN_INFO "can not get VCAM_A_2_8_V.8V\n");
		return -1;
	}
	regulator_set_voltage(VCAM_A_2_8_V, 2800000, 2800000);
	/* ret = regulator_disable(VCAM_A_2_8_V); */

	VCAM_IO_1_8_V = regulator_get(NULL, "hv9");
	if (IS_ERR(VCAM_IO_1_8_V)) {
		printk(KERN_ERR "can not get VCAM_IO_1.8V\n");
		return -1;
	}
	regulator_set_voltage(VCAM_IO_1_8_V, 1800000, 1800000);
	/* ret = regulator_disable(VCAM_IO_1_8_V); */

	gpio_request(CAM_CORE_EN, "cam_1_2v");
	gpio_direction_output(CAM_CORE_EN, 0);

	gpio_request(CAM_AF_EN, "cam_af_2_8v");
	gpio_direction_output(CAM_AF_EN, 0);

	printk(KERN_INFO "set cam_rst cam_stnby  to low\n");
	gpio_request(CAM0_RESET, "cam0_rst");
	gpio_direction_output(CAM0_RESET, 0);

	gpio_request(CAM0_STNBY, "cam0_stnby");
	gpio_direction_output(CAM0_STNBY, 0);

	gpio_request(CAM1_RESET, "cam1_rst");
	gpio_direction_output(CAM1_RESET, 0);

	gpio_request(CAM1_STNBY, "cam1_stnby");
	gpio_direction_output(CAM1_STNBY, 0);

/*	value = ioread32(padctl_base + PADCTRLREG_DCLK1_OFFSET) & (~PADCTRLREG_DCLK1_PINSEL_DCLK1_MASK); */
/*		iowrite32(value, padctl_base + PADCTRLREG_DCLK1_OFFSET); */


	if (on) {
		printk(KERN_INFO "power on the sensor\n"); /* @HW */
		if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_TURBO)) {
			printk(KERN_ERR
				"%s:failed to update dfs request for unicam\n",
				__func__);
			return -1;
		}
		value = clk_enable(axi_clk);
		if (value) {
			printk(KERN_ERR "%s:failed to enable csi2 axi clock\n", __func__);
			return -1;
		}

		msleep(100);
		printk(KERN_INFO "power on the sensor's power supply\n"); /* @HW */

		VCAM_A_2_8_V = regulator_get(NULL, "cam");
		if (IS_ERR(VCAM_A_2_8_V)) {
			printk(KERN_INFO "can not get VCAM_A_2_8_V.8V\n");
			return -1;
		}
		regulator_set_voltage(VCAM_A_2_8_V, 2800000, 2800000);

		regulator_enable(VCAM_A_2_8_V);
		msleep(1);

		VCAM_IO_1_8_V = regulator_get(NULL, "hv9");
		if (IS_ERR(VCAM_IO_1_8_V)) {
			printk(KERN_ERR "can not get VCAM_IO_1.8V\n");
			return -1;
		}
		regulator_set_voltage(VCAM_IO_1_8_V, 1800000, 1800000);
		regulator_enable(VCAM_IO_1_8_V);

		msleep(5);

		value = clk_enable(clock);
		if (value) {
			printk(KERN_ERR "%s: failed to enable clock %s\n", __func__,
				SENSOR_0_CLK);
			return -1;
		}
		printk(KERN_INFO "enable camera clock\n");
		value = clk_set_rate(clock, SENSOR_0_CLK_FREQ);
		if (value) {
			printk(KERN_ERR "%s: failed to set the clock %s to freq %d\n",
					__func__, SENSOR_0_CLK, SENSOR_0_CLK_FREQ);
			return -1;
		}
		printk(KERN_INFO "set rate\n");
		msleep(5);

		gpio_set_value(CAM1_STNBY, 1);
		msleep(35);

		gpio_set_value(CAM1_RESET, 1);
		msleep(2);

		gpio_set_value(CAM1_STNBY, 0);
		msleep(12);

		/*Main Hi-Z*/
		msleep(12);

		gpio_set_value(CAM1_STNBY, 1);
		msleep(12);

		gpio_set_value(CAM1_RESET, 0);
		msleep(1);
		gpio_set_value(CAM1_RESET, 1);

		printk(KERN_INFO "set cam rst to high\n");
		msleep(50);
	} else {
		/* enable reset gpio */
		gpio_set_value(CAM1_RESET, 0);
		msleep(1);

		clk_disable(clock);
		clk_disable(axi_clk);
		msleep(1);

		gpio_set_value(CAM1_STNBY, 0);
		msleep(1);

		/* enable power down gpio */
		regulator_disable(VCAM_IO_1_8_V);
		regulator_disable(VCAM_A_2_8_V);

		if (pi_mgr_dfs_request_update(&unicam_dfs_node,
					      PI_MGR_DFS_MIN_VALUE)) {
			printk(KERN_ERR "%s: failed to update dfs request for unicam\n",
				 __func__);
		}

	}

	return 0;
}
#else
static int sr030pc50_sensor_power(struct device *dev, int on)
{
	printk(KERN_INFO "sr030pc50_sensor_power is empty\n");
}
#endif

bool camdrv_ss_sensor_init_sub(bool bOn, struct camdrv_ss_sensor_cap *sensor)
{
	strcpy(sensor->name, SR030PC50_NAME);
	sensor->supported_preview_framesize_list  = sr030pc50_supported_preview_framesize_list;
	sensor->supported_number_of_preview_sizes = ARRAY_SIZE(sr030pc50_supported_preview_framesize_list);

	sensor->supported_capture_framesize_list  =  sr030pc50_supported_capture_framesize_list;
	sensor->supported_number_of_capture_sizes = ARRAY_SIZE(sr030pc50_supported_capture_framesize_list);

	sensor->fmts			= sr030pc50_fmts;
	sensor->rows_num_fmts		= ARRAY_SIZE(sr030pc50_fmts);

	sensor->controls		= sr030pc50_controls;
	sensor->rows_num_controls	= ARRAY_SIZE(sr030pc50_controls);

	sensor->default_pix_fmt		= SR030PC50_DEFAULT_PIX_FMT;
	sensor->default_mbus_pix_fmt	= SR030PC50_DEFAULT_MBUS_PIX_FMT;
	sensor->register_size		= SR030PC50_REGISTER_SIZE;

	/* sensor dependent functions , Mandatory*/
	sensor->thumbnail_ioctl		= sr030pc50_ss_ioctl;
	sensor->enum_frameintervals	= camdrv_ss_sr030pc50_enum_frameintervals;

	/*REGS and their sizes*/
	/* List all the capabilities of sensor . List all the supported register setting tables */

	sensor->init_regs		= sr030pc50_init_regs;
	sensor->rows_num_init_regs	= ARRAY_SIZE(sr030pc50_init_regs);
	sensor->sensor_power = sr030pc50_sensor_power;
}
