/* Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
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

#ifndef _CAM_CSIPHY_DEV_H_
#define _CAM_CSIPHY_DEV_H_

#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/irqreturn.h>
#include <linux/ion.h>
#include <linux/iommu.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-subdev.h>
#include <media/cam_defs.h>
#include <cam_sensor_cmn_header.h>
#include <cam_req_mgr_interface.h>
#include <cam_subdev.h>
#include <cam_io_util.h>
#include <cam_cpas_api.h>
#include "cam_soc_util.h"
#include "cam_debug_util.h"
#include "cam_context.h"

#define MAX_CSIPHY                  3
#define MAX_DPHY_DATA_LN            4
#define MAX_LRME_V4l2_EVENTS        30
#define CSIPHY_NUM_CLK_MAX          16
#define MAX_CSIPHY_REG_ARRAY        70
#define MAX_CSIPHY_CMN_REG_ARRAY    5

#define MAX_LANES                   5
#define MAX_SETTINGS_PER_LANE       43
#define MAX_DATA_RATES              3
#define MAX_DATA_RATE_REGS          30

#define MAX_REGULATOR         5
#define CAMX_CSIPHY_DEV_NAME "cam-csiphy-driver"

#define CSIPHY_POWER_UP       0
#define CSIPHY_POWER_DOWN     1

#define CSIPHY_DEFAULT_PARAMS            0
#define CSIPHY_LANE_ENABLE               1
#define CSIPHY_SETTLE_CNT_LOWER_BYTE     2
#define CSIPHY_SETTLE_CNT_HIGHER_BYTE    3
#define CSIPHY_DNP_PARAMS                4
#define CSIPHY_2PH_REGS                  5
#define CSIPHY_3PH_REGS                  6

#define CSIPHY_MAX_INSTANCES     2

#define CAM_CSIPHY_MAX_DPHY_LANES    4
#define CAM_CSIPHY_MAX_CPHY_LANES    3

#define ENABLE_IRQ false

#undef CDBG
#ifdef CAM_CSIPHY_CORE_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) pr_debug(fmt, ##args)
#endif

enum cam_csiphy_state {
	CAM_CSIPHY_INIT,
	CAM_CSIPHY_ACQUIRE,
	CAM_CSIPHY_START,
};

/**
 * struct csiphy_reg_parms_t
 * @mipi_csiphy_glbl_irq_cmd_addr: CSIPhy irq addr
 * @mipi_csiphy_interrupt_status0_addr:
 *     CSIPhy interrupt status addr
 * @mipi_csiphy_interrupt_mask0_addr:
 *     CSIPhy interrupt mask addr
 * @mipi_csiphy_interrupt_mask_val:
 *      CSIPhy interrupt mask val
 * @mipi_csiphy_interrupt_clear0_addr:
 *     CSIPhy interrupt clear addr
 * @csiphy_version: CSIPhy Version
 * @csiphy_common_array_size: CSIPhy common array size
 * @csiphy_reset_array_size: CSIPhy reset array size
 * @csiphy_2ph_config_array_size: 2ph settings size
 * @csiphy_3ph_config_array_size: 3ph settings size
 * @csiphy_cpas_cp_bits_per_phy: CP bits per phy
 * @csiphy_cpas_cp_is_interleaved: checks whether cp bits
 *      are interleaved or not
 * @csiphy_cpas_cp_2ph_offset: cp register 2ph offset
 * @csiphy_cpas_cp_3ph_offset: cp register 3ph offset
 * @csiphy_2ph_clock_lane: clock lane in 2ph
 * @csiphy_2ph_combo_ck_ln: clk lane in combo 2ph
 */
struct csiphy_reg_parms_t {
/*MIPI CSI PHY registers*/
	uint32_t mipi_csiphy_glbl_irq_cmd_addr;
	uint32_t mipi_csiphy_interrupt_status0_addr;
	uint32_t mipi_csiphy_interrupt_mask0_addr;
	uint32_t mipi_csiphy_interrupt_mask_val;
	uint32_t mipi_csiphy_interrupt_mask_addr;
	uint32_t mipi_csiphy_interrupt_clear0_addr;
	uint32_t csiphy_version;
	uint32_t csiphy_common_array_size;
	uint32_t csiphy_reset_array_size;
	uint32_t csiphy_2ph_config_array_size;
	uint32_t csiphy_3ph_config_array_size;
	uint32_t csiphy_cpas_cp_bits_per_phy;
	uint32_t csiphy_cpas_cp_is_interleaved;
	uint32_t csiphy_cpas_cp_2ph_offset;
	uint32_t csiphy_cpas_cp_3ph_offset;
	uint32_t csiphy_2ph_clock_lane;
	uint32_t csiphy_2ph_combo_ck_ln;
};

/**
 * struct intf_params
 * @device_hdl: Device Handle
 * @session_hdl: Session Handle
 * @ops: KMD operations
 * @crm_cb: Callback API pointers
 */
struct intf_params {
	int32_t device_hdl[CSIPHY_MAX_INSTANCES];
	int32_t session_hdl[CSIPHY_MAX_INSTANCES];
	int32_t link_hdl[CSIPHY_MAX_INSTANCES];
	struct cam_req_mgr_kmd_ops ops;
	struct cam_req_mgr_crm_cb *crm_cb;
};

/**
 * struct csiphy_reg_t
 * @reg_addr: Register address
 * @reg_data: Register data
 * @delay: Delay
 * @csiphy_param_type: CSIPhy parameter type
 */
struct csiphy_reg_t {
	int32_t  reg_addr;
	int32_t  reg_data;
	int32_t  delay;
	uint32_t csiphy_param_type;
};

struct csiphy_device;

/*
 * struct data_rate_reg_info_t
 * @bandwidth: max bandwidth supported by this reg settings
 * @data_rate_reg_array_size: number of reg value pairs in the array
 * @csiphy_data_rate_regs: array of data rate specific reg value pairs
 */
struct data_rate_reg_info_t {
	uint64_t bandwidth;
	ssize_t  data_rate_reg_array_size;
	struct csiphy_reg_t csiphy_data_rate_regs[MAX_DATA_RATE_REGS];
};

/**
 * struct data_rate_settings_t
 * @num_data_rate_settings: number of valid settings
 *                          present in the data rate settings array
 * @data_rate_settings: array of regsettings which are specific to
 *                      data rate
 */
struct data_rate_settings_t {
	ssize_t num_data_rate_settings;
	struct data_rate_reg_info_t data_rate_settings[MAX_DATA_RATES];
};

/**
 * struct csiphy_ctrl_t
 * @csiphy_reg: Register address
 * @csiphy_common_reg: Common register set
 * @csiphy_reset_reg: Reset register set
 * @csiphy_2ph_reg: 2phase register set
 * @csiphy_2ph_combo_mode_reg:
 *     2phase combo register set
 * @csiphy_3ph_reg: 3phase register set
 * @csiphy_2ph_3ph_mode_reg:
 *     2 phase 3phase combo register set
 * @getclockvoting: function pointer which
 *      is used to find the clock voting
 *      for the sensor output data rate
 * @data_rate_settings_table:
 *      Table which maintains the resgister
 *      settings specific to data rate
 */
struct csiphy_ctrl_t {
	struct csiphy_reg_parms_t csiphy_reg;
	struct csiphy_reg_t *csiphy_common_reg;
	struct csiphy_reg_t *csiphy_irq_reg;
	struct csiphy_reg_t *csiphy_reset_reg;
	struct csiphy_reg_t (*csiphy_2ph_reg)[MAX_SETTINGS_PER_LANE];
	struct csiphy_reg_t (*csiphy_2ph_combo_mode_reg)[MAX_SETTINGS_PER_LANE];
	struct csiphy_reg_t (*csiphy_3ph_reg)[MAX_SETTINGS_PER_LANE];
	struct csiphy_reg_t (*csiphy_2ph_3ph_mode_reg)[MAX_SETTINGS_PER_LANE];
	enum   cam_vote_level (*getclockvoting)(struct csiphy_device *phy_dev);
	struct data_rate_settings_t *data_rates_settings_table;
};

/**
 * cam_csiphy_param: Provides cmdbuffer structre
 * @lane_mask     :  Lane mask details
 * @lane_assign   :  Lane sensor will be using
 * @csiphy_3phase :  Mentions DPHY or CPHY
 * @combo_mode    :  Info regarding combo_mode is enable / disable
 * @lane_cnt      :  Total number of lanes
 * @reserved
 * @3phase        :  Details whether 3Phase / 2Phase operation
 * @settle_time   :  Settling time in ms
 * @settle_time_combo_sensor   :  Settling time in ms
 * @data_rate     :  Data rate in mbps
 * @data_rate_combo_sensor: data rate of combo sensor
 *                          in the the same phy
 *
 */
struct cam_csiphy_param {
	uint16_t    lane_mask;
	uint16_t    lane_assign;
	uint8_t     csiphy_3phase;
	uint8_t     combo_mode;
	uint8_t     lane_cnt;
	uint8_t     secure_mode[CSIPHY_MAX_INSTANCES];
	uint64_t    settle_time;
	uint64_t    settle_time_combo_sensor;
	uint64_t    data_rate;
	uint64_t    data_rate_combo_sensor;
};

/**
 * struct csiphy_device
 * @device_name: Device name
 * @pdev: Platform device
 * @irq: Interrupt structure
 * @base: Base address
 * @hw_version: Hardware Version
 * @csiphy_state: CSIPhy state
 * @ctrl_reg: CSIPhy control registers
 * @num_clk: Number of clocks
 * @csiphy_max_clk: Max timer clock rate
 * @num_vreg: Number of regulators
 * @csiphy_clk: Clock structure
 * @csiphy_clk_info: Clock information structure
 * @csiphy_vreg: Regulator structure
 * @csiphy_reg_ptr: Regulator structure
 * @csiphy_3p_clk_info: 3Phase clock information
 * @csiphy_3p_clk: 3Phase clocks structure
 * @csi_3phase: Is it a 3Phase mode
 * @ref_count: Reference count
 * @clk_lane: Clock lane
 * @acquire_count: Acquire device count
 * @start_dev_count: Start count
 * @is_acquired_dev_combo_mode:
 *    Flag that mentions whether already acquired
 *   device is for combo mode
 * @soc_info: SOC information
 * @cpas_handle: CPAS handle
 * @config_count: Config reg count
 * @csiphy_cpas_cp_reg_mask: CP reg mask for phy instance
 */
struct csiphy_device {
	char device_name[CAM_CTX_DEV_NAME_MAX_LENGTH];
	struct mutex mutex;
	uint32_t hw_version;
	enum cam_csiphy_state csiphy_state;
	struct csiphy_ctrl_t *ctrl_reg;
	uint32_t csiphy_max_clk;
	struct msm_cam_clk_info csiphy_3p_clk_info[2];
	struct clk *csiphy_3p_clk[2];
	unsigned char csi_3phase;
	int32_t ref_count;
	uint16_t lane_mask[MAX_CSIPHY];
	uint8_t is_csiphy_3phase_hw;
	uint8_t num_irq_registers;
	struct cam_subdev v4l2_dev_str;
	struct cam_csiphy_param csiphy_info;
	struct intf_params bridge_intf;
	uint32_t clk_lane;
	uint32_t acquire_count;
	uint32_t start_dev_count;
	uint32_t is_acquired_dev_combo_mode;
	struct cam_hw_soc_info   soc_info;
	uint32_t cpas_handle;
	uint32_t config_count;
	uint64_t csiphy_cpas_cp_reg_mask[CSIPHY_MAX_INSTANCES];
};

#endif /* _CAM_CSIPHY_DEV_H_ */
