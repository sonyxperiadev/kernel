/* Copyright (c) 2013-2016, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __MSM_ISP_UTIL_H__
#define __MSM_ISP_UTIL_H__

#include "msm_isp.h"
#include <soc/qcom/camera2.h>

/* #define CONFIG_MSM_ISP_DBG 1 */

#ifdef CONFIG_MSM_ISP_DBG
#define ISP_DBG(fmt, args...) printk(fmt, ##args)
#else
#define ISP_DBG(fmt, args...) pr_debug(fmt, ##args)
#endif

#define ALT_VECTOR_IDX(x) {x = 3 - x; }

struct msm_isp_bandwidth_mgr {
	uint32_t bus_client;
	uint32_t bus_vector_active_idx;
	uint32_t use_count;
	struct msm_isp_bandwidth_info client_info[MAX_ISP_CLIENT];
};

uint32_t msm_isp_get_framedrop_period(
	enum msm_vfe_frame_skip_pattern frame_skip_pattern);
void msm_isp_reset_burst_count_and_frame_drop(
	struct vfe_device *vfe_dev, struct msm_vfe_axi_stream *stream_info);

int msm_isp_init_bandwidth_mgr(enum msm_isp_hw_client client);
int msm_isp_update_bandwidth(enum msm_isp_hw_client client,
	uint64_t ab, uint64_t ib);
void msm_isp_util_get_bandwidth_stats(struct vfe_device *vfe_dev,
				      struct msm_isp_statistics *stats);
void msm_isp_util_update_last_overflow_ab_ib(struct vfe_device *vfe_dev);
void msm_isp_util_update_clk_rate(long clock_rate);
void msm_isp_update_req_history(uint32_t client, uint64_t ab,
				uint64_t ib,
				struct msm_isp_bandwidth_info *client_info,
				unsigned long long ts);
void msm_isp_deinit_bandwidth_mgr(enum msm_isp_hw_client client);

int msm_isp_subscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
	struct v4l2_event_subscription *sub);

int msm_isp_unsubscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
	struct v4l2_event_subscription *sub);

int msm_isp_proc_cmd(struct vfe_device *vfe_dev, void *arg);
int msm_isp_send_event(struct vfe_device *vfe_dev,
	uint32_t type, struct msm_isp_event_data *event_data);
int msm_isp_cal_word_per_line(uint32_t output_format,
	uint32_t pixel_per_line);
int msm_isp_get_bit_per_pixel(uint32_t output_format);
enum msm_isp_pack_fmt msm_isp_get_pack_format(uint32_t output_format);
irqreturn_t msm_isp_process_irq(int irq_num, void *data);
int msm_isp_set_src_state(struct vfe_device *vfe_dev, void *arg);
void msm_isp_do_tasklet(unsigned long data);
void msm_isp_update_error_frame_count(struct vfe_device *vfe_dev);
void msm_isp_process_error_info(struct vfe_device *vfe_dev);
int msm_isp_open_node(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh);
int msm_isp_close_node(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh);
long msm_isp_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg);
int msm_isp_get_clk_info(struct vfe_device *vfe_dev,
	struct platform_device *pdev, struct msm_cam_clk_info *vfe_clk_info);
void msm_isp_fetch_engine_done_notify(struct vfe_device *vfe_dev,
	struct msm_vfe_fetch_engine_info *fetch_engine_info);
void msm_camera_io_dump_2(void __iomem *addr, int size);
void msm_isp_print_fourcc_error(const char *origin, uint32_t fourcc_format);

#endif /* __MSM_ISP_UTIL_H__ */
