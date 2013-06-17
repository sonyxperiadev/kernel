/*****************************************************************************
* Copyright 2006 - 2009 Broadcom Corporation.  All rights reserved.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>
#include <media/kona-unicam.h>

#include "unicam.h"
#include "unicam_csi2.h"

#define UNICAM_CAPTURE_MODE		CSL_CAM_CAPTURE_MODE_TRIGGER

static const unsigned int csi2_input_fmts[] = {
	V4L2_MBUS_FMT_UYVY8_2X8,
	V4L2_MBUS_FMT_YUYV8_2X8,
};

static int unicam_csi2_update_buf(struct unicam_csi2_device *csi2)
{
	CSL_CAM_BUFFER_st_t cslCamBuffer0;
	CSL_CAM_BUFFER_st_t cslCamBufferData0;

	if (!csi2->active_buf)
		return 0;

	/* image 0 */
	cslCamBuffer0.start_addr = (UInt32) csi2->active_buf->unicam_addr;
	cslCamBuffer0.line_stride = (UInt32) csi2->video_out.bpl_value;
	cslCamBuffer0.size =
		(csi2->video_out.bpl_value * csi2->video_out.height);
	cslCamBuffer0.buffer_wrap_en = 1;
	cslCamBuffer0.mem_type = CSL_CAM_MEM_TYPE_NONE;


	/* buffer 0  for now no embedded data */
	cslCamBufferData0.start_addr = (UInt32) 0;
	cslCamBufferData0.line_stride = 0;
	cslCamBufferData0.buffer_wrap_en = 0;
	cslCamBufferData0.size = 0;
	cslCamBufferData0.mem_type = cslCamBuffer0.mem_type;

	if (csl_cam_set_input_addr
			(csi2->cslCamHandle, &cslCamBuffer0, NULL,
			 &cslCamBufferData0)) {

		dev_err(csi2->unicam->dev,
				"CamCslSetInputIntf(): csl_cam_set_input_addr(): error\n");
		return -EINVAL;
	}

	return 0;
}

static int unicam_csi2_capture(struct unicam_csi2_device *csi2)
{
	CSL_CAM_FRAME_st_t cslCamFrame;

	if (!csi2->active_buf)
		return 0;

	/* enable frame Interrupts */
	cslCamFrame.int_enable = CSL_CAM_INT_FRAME_END;
	/* for testing enabled frame start interrupt */
	/* cslCamFrame.int_enable |= CSL_CAM_INT_FRAME_START;*/
	cslCamFrame.int_line_count = 0;
	cslCamFrame.capture_mode = UNICAM_CAPTURE_MODE;
	cslCamFrame.capture_size = 0;
	if (csl_cam_set_frame_control(csi2->cslCamHandle, &cslCamFrame)) {
		dev_err(csi2->unicam->dev,
			"CSI2: csl_cam_set_frame_control(): error\n");
		return -EINVAL;
	}

	if (UNICAM_CAPTURE_MODE == CSL_CAM_CAPTURE_MODE_TRIGGER) {
		if (csl_cam_trigger_capture(csi2->cslCamHandle) != 0) {
			dev_err(csi2->unicam->dev,
				"CSI2: error in triggering capture: error\n");
			return -EINVAL;
		}
	}

	return 0;
}

void kona_unicam_csi2_isr(struct unicam_csi2_device *csi2)
{
	/* struct unicam_pipeline *pipe =
	 * to_unicam_pipeline(&csi2->subdev.entity);*/
	unsigned int status;
	unsigned int reg_status;
	int ret;

	reg_status =
	    csl_cam_get_rx_status(csi2->cslCamHandle,
				  (CSL_CAM_RX_STATUS_t *) &status);

	if (status & CSL_CAM_RX_INT) {

		/* get and clear interrupt status */
		reg_status =
		    csl_cam_get_intr_status(csi2->cslCamHandle,
					    (CSL_CAM_INTERRUPT_t *) &status);

		if (status & CSL_CAM_INT_FRAME_END) {

			if (likely(csi2->frame_skip <= 0)) {
				csi2->active_buf =
				kona_unicam_video_buffer_next(&csi2->video_out);
			} else
				csi2->frame_skip--;

			ret = unicam_csi2_update_buf(csi2);
			if (ret)  {
				dev_warn(csi2->unicam->dev,
						"CSI2: failed to queue buffer\n");
			}

			ret = unicam_csi2_capture(csi2);
			if (ret) {
				dev_warn(csi2->unicam->dev,
						"CSI2: failed to trigger capture\n");
			}
		}
	}
	/* handle error condition */
}

/* ------------------------------------------------
 * Video operations
 * ------------------------------------------------
 */
static int csi2_queue(struct unicam_video *video, struct unicam_buffer *buffer)
{
	struct unicam_csi2_device *csi2 = container_of(video,
			struct unicam_csi2_device, video_out);

	csi2->active_buf = buffer;

	unicam_csi2_update_buf(csi2);

	unicam_csi2_capture(csi2);

	return 0;
}

static const struct unicam_video_operations csi2_unicamvideo_ops = {
	.queue = csi2_queue,
};

/* -------------------------------------------------
 * Media entity operations
 * -------------------------------------------------
 */
/*
 * csi2_link_setup - setup csi2 connections.
 * @entity	: pointer to media entity structure
 * @local	: pointer to local pad array
 * @remote	: pointer to remote pad array
 * @flags	: link flags
 * return -EINVAL or zero on success
 */
static int csi2_link_setup(struct media_entity *entity,
		const struct media_pad *local,
		const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations csi2_media_ops = {
	.link_setup = csi2_link_setup,
};

/* -------------------------------------------------
 *  Subdev related operations
 * -------------------------------------------------
 */

static struct v4l2_mbus_framefmt *
__csi2_get_format(struct unicam_csi2_device *csi2, struct v4l2_subdev_fh *fh,
		unsigned int pad, enum v4l2_subdev_format_whence which)
{
	if (which == V4L2_SUBDEV_FORMAT_TRY)
		return v4l2_subdev_get_try_format(fh, pad);
	else
		return &csi2->formats[pad];
}

static void
csi2_try_format(struct unicam_csi2_device *csi2, struct v4l2_subdev_fh *fh,
		unsigned int pad, struct v4l2_mbus_framefmt *fmt,
		enum v4l2_subdev_format_whence which)
{
	struct v4l2_mbus_framefmt *format;
	unsigned int i;

	switch (pad) {
	case CSI2_PAD_SINK:
		for (i = 0; i < ARRAY_SIZE(csi2_input_fmts); i++) {
			if (fmt->code == csi2_input_fmts[i])
				break;
		}

		if (i >= ARRAY_SIZE(csi2_input_fmts))
			fmt->code = V4L2_MBUS_FMT_UYVY8_2X8;
		fmt->width = clamp_t(u32, fmt->width, 1, 8191);
		fmt->height = clamp_t(u32, fmt->height, 1, 8191);
		break;

	case CSI2_PAD_SOURCE:
		format = __csi2_get_format(csi2, fh, CSI2_PAD_SINK, which);
		memcpy(fmt, format, sizeof(*fmt));
		break;
	}

	fmt->colorspace = V4L2_COLORSPACE_JPEG;
	fmt->field = V4L2_FIELD_NONE;
}

/*
 * csi2_set_stream - enable/disable streaming on csi2
 * @sd: unicam csi2 v4l2 subdevice
 * @enable: unicam pipeline stream state
 *
 * return 0 on success or a negative error code otherwise
 */

static int csi2_set_stream(struct v4l2_subdev *sd, int enable)
{
	struct unicam_csi2_device *csi2 = v4l2_get_subdevdata(sd);
	struct unicam_device *unicam = csi2->unicam;
	struct unicam_pipeline *pipe = to_unicam_pipeline(&csi2->subdev.entity);
	int ret;

	if (csi2->state == UNICAM_PIPELINE_STREAM_STOPPED) {
		if (enable == UNICAM_PIPELINE_STREAM_STOPPED) {
			dev_warn(unicam->dev,
					"stream already stopped\n");
			return 0;
		}
	}

	switch (enable) {
	case UNICAM_PIPELINE_STREAM_CONTINUOUS: {
		struct media_pad *remote;
		struct unicam_v4l2_subdevs_groups *subdevs;
		CSL_CAM_INTF_CFG_st_t csl_cam_intf_cfg_st;
		CSL_CAM_LANE_CONTROL_st_t cslCamLaneCtrl_st;
		CSL_CAM_PIPELINE_st_t cslCamPipeline;
		CSL_CAM_IMAGE_ID_st_t cslCamImageCtrl;
		CSL_CAM_DATA_st_t cslCamDataCtrl;


		remote = media_entity_remote_source(&csi2->pads[CSI2_PAD_SINK]);
		if (remote == NULL || media_entity_type(remote->entity) !=
			MEDIA_ENT_T_V4L2_SUBDEV) {
			dev_err(unicam->dev, "sensor link not setup correctly\n");
			return -EINVAL;
		}

		pipe->external = media_entity_to_v4l2_subdev(remote->entity);
		if (pipe->external == NULL) {
			dev_err(unicam->dev, "subdev set to null\n");
			return -ENODEV;
		}

		subdevs = pipe->external->host_priv;

		csi2->frame_skip = 0;
		v4l2_subdev_call(pipe->external, sensor, g_skip_frames,
				&csi2->frame_skip);

		ret = csl_cam_init();
		if (ret < 0) {
			dev_err(unicam->dev, "error initializing csl camera\n");
			return -EINVAL;
		}

		/* set camera interface parameters */
		memset(&csl_cam_intf_cfg_st, 0, sizeof(CSL_CAM_INTF_CFG_st_t));

		if (subdevs->interface == UNICAM_INTERFACE_CSI2_PHY1)
			csl_cam_intf_cfg_st.intf = CSL_CAM_INTF_CSI;
		else {
			dev_err(unicam->dev, "CSI2 PHY1 only supported\n");
			return -EINVAL;
		}

		if (subdevs->bus.csi2.port == UNICAM_PORT_AFE_0)
			csl_cam_intf_cfg_st.afe_port = CSL_CAM_PORT_AFE_0;
		else if (subdevs->bus.csi2.port == UNICAM_PORT_AFE_1)
			csl_cam_intf_cfg_st.afe_port = CSL_CAM_PORT_AFE_1;
		else {
			dev_err(unicam->dev, "CSI2 only supports two port\n");
			return -EINVAL;
		}

		if (subdevs->bus.csi2.lanes == CSI2_SINGLE_LANE_SENSOR)
			csl_cam_intf_cfg_st.input_mode =
				CSL_CAM_INPUT_SINGLE_LANE;
		else if (subdevs->bus.csi2.lanes == CSI2_DUAL_LANE_SENSOR)
			csl_cam_intf_cfg_st.input_mode =
				CSL_CAM_INPUT_DUAL_LANE;
		else {
			dev_err(unicam->dev, "CSI2 only max two lanes\n");
			return -EINVAL;
		}

		csl_cam_intf_cfg_st.frame_time_out = 1000;
		csl_cam_intf_cfg_st.p_cpi_intf_st = NULL;

		/* open camera interface */
		if (csl_cam_open(&csl_cam_intf_cfg_st,
					&csi2->cslCamHandle)) {
			dev_err(unicam->dev, "CSI2 csl_cam_open(): error\n");
			return -EINVAL;
		}

		/* set data lane timing */
		cslCamLaneCtrl_st.lane_select = CSL_CAM_DATA_LANE_0;
		cslCamLaneCtrl_st.lane_control = CSL_CAM_LANE_HS_TERM_TIME;
		/*TODO fix this implement ext ctrl */
		cslCamLaneCtrl_st.param = 1;
		if (csl_cam_set_lane_control
				(csi2->cslCamHandle, &cslCamLaneCtrl_st)) {
			dev_err(unicam->dev,
					"CSI2: csl_cam_set_lane_control(): error\n");
			return -EINVAL;
		}

		/* pipelince decode */
		cslCamPipeline.decode = CSL_CAM_DEC_NONE;
		cslCamPipeline.unpack = CSL_CAM_PIXEL_NONE;
		cslCamPipeline.pack = CSL_CAM_PIXEL_NONE;
		cslCamPipeline.dec_adv_predictor = FALSE;

		cslCamPipeline.encode = CSL_CAM_ENC_NONE;
		cslCamPipeline.enc_adv_predictor = FALSE;
		cslCamPipeline.encode_blk_size = 0x0000;

		/* set pipeline */
		if (csl_cam_set_pipeline_control
				(csi2->cslCamHandle, &cslCamPipeline)) {
			dev_err(unicam->dev,
					"CSI2: csl_cam_set_pipeline_control(): error\n");
			return -EINVAL;
		}

		/* set image identifier (CSI mode only) */
		memset(&cslCamImageCtrl, 0, sizeof(CSL_CAM_IMAGE_ID_st_t));
		/*TODO should be set according to format */
		cslCamImageCtrl.image_data_id0 = 0x1E;
		if (csl_cam_set_image_type_control
				(csi2->cslCamHandle, &cslCamImageCtrl)) {
			dev_err(unicam->dev,
					"CSI2: csl_cam_set_image_type_control(): errror\n");
			return -EINVAL;
		}

		/* set data capture */
		cslCamDataCtrl.int_enable =
			(CSL_CAM_INTERRUPT_t) (CSL_CAM_INT_DISABLE);
		cslCamDataCtrl.line_count = 2;
		cslCamDataCtrl.data_id = 0x00;
		cslCamDataCtrl.data_size = CSL_CAM_PIXEL_8BIT;
		cslCamDataCtrl.fsp_decode_enable = FALSE;

		if (csl_cam_set_data_type_control
				(csi2->cslCamHandle, &cslCamDataCtrl)) {
			dev_err(unicam->dev,
					"CSI2: csl_cam_set_data_type_control(): error\n");
			return -EINVAL;
		}

		/* start receiver */
		if (csl_cam_rx_start(csi2->cslCamHandle)) {
			dev_err(csi2->unicam->dev,
					"CSI2: csl_cam_rx_start(): error\n");
			return -EINVAL;
		}
		ret = 0;
		break;
	}
	case UNICAM_PIPELINE_STREAM_STOPPED: {
		CSL_CAM_FRAME_st_t cslCamFrame;

		 /* disable frame interrupts */
		cslCamFrame.int_enable = CSL_CAM_INT_DISABLE;
		cslCamFrame.int_line_count = 0;
		cslCamFrame.capture_mode = UNICAM_CAPTURE_MODE;
		cslCamFrame.capture_size = 0;

		if (csl_cam_set_frame_control(
					csi2->cslCamHandle, &cslCamFrame)) {
			dev_err(unicam->dev,
					"CSI2: csl_cam_set_frame_control(): error\n");
			ret = -EINVAL;
		}

		/* disable receiver */
		if (csl_cam_rx_stop(csi2->cslCamHandle)) {
			dev_err(unicam->dev,
					"CSI2: csl_cam_rx_stop(): error\n");
			ret = -EINVAL;
		}

		if (csl_cam_close(csi2->cslCamHandle)) {
			dev_err(unicam->dev,
					"CSI2: csl_cam_close(): error\n");
			ret = -EINVAL;
		}

		if (csl_cam_exit()) {
			dev_err(unicam->dev,
					"CSI2: csl_cam_exit(): FAILED\n");
			ret = -EINVAL;
		}
		break;
	}
	}

	csi2->active_buf = NULL;
	csi2->state = enable;
	return 0;
}

/*
 * csi2_enum_mbus_code - handle pixel format enumeration
 * @sd:	pointer to v4l2 subdev structure
 * @fh: v4l2 subdev file handle
 * @code: pointer to v4l2_subdev_mbus_code_enum structure
 * return -EINVAL or zero on success
 */
static int csi2_enum_mbus_code(struct v4l2_subdev *sd,
		struct v4l2_subdev_fh *fh,
		struct v4l2_subdev_mbus_code_enum *code)
{
	return 0;
}

/*
 * csi2_enum_frame_size - enumerate frame sizes
 * @sd: pointer to v4l2 subdev structure
 * @fh: v4l2 subdev file handle
 * @fse: pointer to v4l2_subdev_frame_size_enum structure
 * return -EINVAL or zero on succes
 */
static int csi2_enum_frame_size(struct v4l2_subdev *sd,
		struct v4l2_subdev_fh *fh,
		struct v4l2_subdev_frame_size_enum *fse)
{
	struct unicam_csi2_device *csi2 = v4l2_get_subdevdata(sd);
	struct v4l2_mbus_framefmt format;

	if (fse->index != 0)
		return -EINVAL;

	format.code = fse->code;
	format.width = 1;
	format.height = 1;
	csi2_try_format(csi2, fh, fse->pad, &format, V4L2_SUBDEV_FORMAT_TRY);
	fse->min_width = format.width;
	fse->min_height = format.height;

	if (format.code != fse->code)
		return -EINVAL;

	format.code = fse->code;
	format.width = -1;
	format.height = -1;
	csi2_try_format(csi2, fh, fse->pad, &format, V4L2_SUBDEV_FORMAT_TRY);
	fse->max_width = format.width;
	fse->max_height = format.height;

	return 0;
}

/*
 * csi2_get_format - handle get format by pads subdev method
 * @sd: pointer to v4l2 subdev structure
 * @fh: v4l2 subdev file handle
 * @fmt: pointer to v4l2 subdev format structure
 * return -EINVAL or zero on success
 */
static int csi2_get_format(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
		struct v4l2_subdev_format *fmt)
{
	struct unicam_csi2_device *csi2 = v4l2_get_subdevdata(sd);
	struct v4l2_mbus_framefmt *format;

	format = __csi2_get_format(csi2, fh, fmt->pad, fmt->which);
	if (format == NULL)
		return -EINVAL;

	fmt->format = *format;

	return 0;
}

/*
 * csi2_set_format - handle set format by pads subdev method
 * @sd: pointer to v4l2 subdev structure
 * @fh: v4l2 subdev file handle
 * @fmt: pointer to v4l2 subdev format structure
 * return - EINVAL or zero on success
 */
static int csi2_set_format(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
		struct v4l2_subdev_format *fmt)
{
	struct unicam_csi2_device *csi2 = v4l2_get_subdevdata(sd);
	struct v4l2_mbus_framefmt *format;

	format = __csi2_get_format(csi2, fh, fmt->pad, fmt->which);
	if (format == NULL)
		return -EINVAL;

	csi2_try_format(csi2, fh, fmt->pad, &fmt->format, fmt->which);
	*format = fmt->format;

	if (fmt->pad == CSI2_PAD_SINK) {
		format = __csi2_get_format(csi2, fh, CSI2_PAD_SOURCE,
				fmt->which);
		*format = fmt->format;
		csi2_try_format(csi2, fh, CSI2_PAD_SOURCE, format, fmt->which);
	}

	return 0;
}

/*
 * csi2_init_formats - Initialize formats on all pads
 * @sd: unicam csi2 v4l2 subdevice
 * @fh: V4L2 subdev file handle
 *
 * Initialize all pad formats with default values. if fh is not null, try
 * formats are initialize on the file handle otherwise active formats are
 * initialized on the device
 */
static int csi2_init_formats(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct v4l2_subdev_format format;
	memset(&format, 0, sizeof(format));
	format.pad = CSI2_PAD_SINK;
	format.which = fh ? V4L2_SUBDEV_FORMAT_TRY : V4L2_SUBDEV_FORMAT_ACTIVE;
	format.format.code = V4L2_MBUS_FMT_UYVY8_2X8;
	format.format.width = 4096;
	format.format.height = 4096;
	csi2_set_format(sd, fh, &format);
	return 0;
}


/* subdev video operations */
static const struct v4l2_subdev_video_ops csi2_video_ops = {
	.s_stream = csi2_set_stream,
};

/* subdev pad operations */
static const struct v4l2_subdev_pad_ops csi2_pad_ops = {
	.enum_mbus_code = csi2_enum_mbus_code,
	.enum_frame_size = csi2_enum_frame_size,
	.get_fmt = csi2_get_format,
	.set_fmt = csi2_set_format,
};

/* subdev operations */
static const struct v4l2_subdev_ops csi2_ops = {
	.video = &csi2_video_ops,
	.pad = &csi2_pad_ops,

};

/* subdev internal operations */
static const struct v4l2_subdev_internal_ops csi2_internal_ops = {
	.open = csi2_init_formats,
};

/*
 * csi2_init_entities - Initialize subdev and media entry
 * @csi2: pointer to csi2 structure
 * return -ENOMEM or zero on success;
 */
static int csi2_init_entities(struct unicam_csi2_device *csi2,
		const char *subname)
{
	struct v4l2_subdev *sd = &csi2->subdev;
	struct media_pad *pads = csi2->pads;
	struct media_entity *me = &sd->entity;
	int ret;
	char name[V4L2_SUBDEV_NAME_SIZE];

	v4l2_subdev_init(sd, &csi2_ops);
	sd->internal_ops = &csi2_internal_ops;
	snprintf(name, sizeof(name), "CSI2%s", subname);
	strlcpy(sd->name, "", sizeof(sd->name));
	sprintf(sd->name, "KONA UNICAM %s", name);

	sd->grp_id = 1 << 16; /* grouping unicam subdevs */
	v4l2_set_subdevdata(sd, csi2);
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE; /* create a device node */

	pads[CSI2_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE;
	pads[CSI2_PAD_SINK].flags = MEDIA_PAD_FL_SINK;

	me->ops = &csi2_media_ops;
	/* one extra link, since we can support two sensors */
	ret = media_entity_init(me, CSI2_PADS_NUM, pads, 1);
	if (ret < 0) {
		dev_err(csi2->unicam->dev, "failed to initialize media entity\n");
		return ret;
	}

	csi2_init_formats(sd, NULL);

	csi2->video_out.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	csi2->video_out.ops = &csi2_unicamvideo_ops;
	csi2->video_out.bpl_alignment = 16;
	csi2->video_out.bpl_zero_padding = 0;
	csi2->video_out.bpl_max = 0x1ffe0;
	csi2->video_out.unicam = csi2->unicam;

	ret = kona_unicam_video_init(&csi2->video_out, name);
	if (ret < 0)
		goto error_video;

	/* connect csi2 subdev to the video node */
	ret = media_entity_create_link(&csi2->subdev.entity, CSI2_PAD_SOURCE,
			&csi2->video_out.video.entity, 0, 0);

	if (ret < 0)
		goto error_link;

	return 0;
error_link:
	kona_unicam_video_cleanup(&csi2->video_out);
error_video:
	media_entity_cleanup(&csi2->subdev.entity);
	return ret;
}

/* ------------------------------------------------------
 *  Operations called by unicam core for intialization
 * ------------------------------------------------------
 */

void kona_unicam_csi2_unregister_entities(struct unicam_csi2_device *csi2)
{
	v4l2_device_unregister_subdev(&csi2->subdev);
	kona_unicam_video_unregister(&csi2->video_out);
}

int kona_unicam_csi2_register_entities(struct unicam_csi2_device *csi2,
		struct v4l2_device *vdev)
{
	int ret;

	/* register the subdev and video nodes, this also registers
	 * the entities with media device pointed by vdev
	 */
	ret = v4l2_device_register_subdev(vdev, &csi2->subdev);
	if (ret < 0) {
		dev_err(csi2->unicam->dev,
				"failed to register csi2 v4l2 subdevice\n");
		return ret;
	}

	ret = kona_unicam_video_register(&csi2->video_out, vdev);
	if (ret < 0)
		goto error;

	return 0;

error:
	kona_unicam_csi2_unregister_entities(csi2);
	return ret;
}

/*
 * kona_unicam_csi2_cleanup - Routine for module driver cleanup
 */
void kona_unicam_csi2_cleanup(struct unicam_device *unicam)
{
	struct unicam_csi2_device *csi2a = &unicam->csi2a;

	kona_unicam_video_cleanup(&csi2a->video_out);
	media_entity_cleanup(&csi2a->subdev.entity);
}

/*
 * kona_unicam_csi2_init - Routine for module driver init
 */
int kona_unicam_csi2_init(struct unicam_device *unicam)
{
	struct unicam_csi2_device *csi2a = &unicam->csi2a;
	int ret;

	csi2a->unicam = unicam;
	csi2a->state = UNICAM_PIPELINE_STREAM_STOPPED;

	ret = csi2_init_entities(csi2a, "a");

	return ret;
}
